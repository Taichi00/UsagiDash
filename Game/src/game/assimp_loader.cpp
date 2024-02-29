#include "game/assimp_loader.h"
#include "engine/engine.h"
#include "engine/shared_struct.h"
#include "game/resource/texture2d.h"
#include "engine/vertex_buffer.h"
#include "engine/index_buffer.h"
#include "engine/pipeline_state.h"
#include "engine/descriptor_heap.h"
#include "math/easing.h"
#include "game/game.h"
#include "game/bone.h"
#include "game/bone_list.h"
#include "game/animation.h"
#include "math/vec.h"
#include "math/quaternion.h"
#include "game/resource/model.h"
#include "game/resource/collision_model.h"
#include "game/string_methods.h"
#include "math/aabb.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/material.h>
#include <assimp/pbrmaterial.h>
#include "engine/d3dx12.h"
#include <filesystem>
#include <d3d12.h>
#include <stdio.h>
#include <utility>
#include <map>
#include <set>
#include <DirectXMath.h>

namespace fs = std::filesystem;

std::wstring AssimpLoader::directory_path;

std::unique_ptr<Model> AssimpLoader::Load(const std::wstring& path)
{
    if (path.empty())
        return nullptr;

    // モデルのインスタンスを生成
    auto model = std::make_unique<Model>();
    auto& meshes = model->meshes;
    auto& materials = model->materials;
    auto& bones = model->bones;
    auto& animations = model->animations;

    directory_path = StringMethods::GetDirectoryPath(path);

    // インポートフラグを設定
    int flag = 0;
    flag |= aiProcess_Triangulate;
    flag |= aiProcess_LimitBoneWeights;
    flag |= aiProcess_FlipUVs;
    flag |= aiProcess_GenUVCoords;
    flag |= aiProcess_CalcTangentSpace;
    
    // ファイルを読み込む
    Assimp::Importer importer;
    auto scene = importer.ReadFile(StringMethods::GetString(path), flag);
    
    if (!scene)
    {
        // もし読み込みエラーが出たら表示する
        printf(importer.GetErrorString());
        printf("\n");
        return nullptr;
    }
    
    meshes.resize(scene->mNumMeshes);
    materials.resize(scene->mNumMaterials);
    animations.resize(scene->mNumAnimations);

    // bone tree を構築
    CreateBoneTree(bones, nullptr, scene->mRootNode);
    
    // Mesh, Boneの読み込み
    ProcessNode(*model, scene->mRootNode, scene);

    // Materialの読み込み
    for (int i = 0; i < materials.size(); i++)
    {
        const auto aimaterial = scene->mMaterials[i];
        LoadMaterial(materials[i], aimaterial, scene);
    }

    // Aniamtionの読み込み
    for (int i = 0; i < animations.size(); i++)
    {
        animations[i] = std::make_unique<Animation>();
        LoadAnimation(*animations[i], scene->mAnimations[i], bones);
    }

    // SmoothNormalの生成（アウトライン用）
    flag |= aiProcess_GenSmoothNormals;
    flag |= aiProcess_ForceGenNormals;
    scene = importer.ReadFile(StringMethods::GetString(path), flag);

    for (int i = 0; i < meshes.size(); i++)
    {
        auto& mesh = meshes[i];
        auto aimesh = scene->mMeshes[i];
        for (int j = 0; j < mesh.vertices.size(); j++)
        {
            auto vertex = &(mesh.vertices[j]);
            auto normal = &(aimesh->mNormals[j]);

            vertex->smooth_normal = DirectX::XMFLOAT3(normal->x, normal->y, normal->z);
        }
    }

    // 頂点バッファ・インデックスバッファの生成
    for (int i = 0; i < meshes.size(); i++)
    {
        meshes[i].vertex_buffer = Game::Get()->GetEngine()->CreateVertexBuffer(meshes[i].vertices);
        meshes[i].index_buffer = Game::Get()->GetEngine()->CreateIndexBuffer(meshes[i].indices);
    }

    // ディスクリプタヒープの生成
    model->descriptor_heap = Game::Get()->GetEngine()->CreateDescriptorHeap((unsigned int)(materials.size() * 3));

    // ディスクリプタハンドルの取得、SRVの生成
    for (auto& mat : materials)
    {
        mat.albedo_handle = model->descriptor_heap->Alloc();
        mat.pbr_handle = model->descriptor_heap->Alloc();
        mat.normal_handle = model->descriptor_heap->Alloc();

        Game::Get()->GetEngine()->CreateShaderResourceView(*mat.albedo_texture, mat.albedo_handle);
        Game::Get()->GetEngine()->CreateShaderResourceView(*mat.pbr_texture, mat.pbr_handle);
        Game::Get()->GetEngine()->CreateShaderResourceView(*mat.normal_texture, mat.normal_handle);
    }

    // AABBの設定
    if (!meshes.empty())
    {
        model->aabb = meshes[0].aabb;
    }
    for (auto i = 1; i < meshes.size(); i++)
    {
        model->aabb.max = Vec3::Max(model->aabb.max, meshes[i].aabb.max);
        model->aabb.min = Vec3::Min(model->aabb.min, meshes[i].aabb.min);
    }

    // ボーン情報の表示
    /*for (int i = 0; i < bones.Size(); i++)
    {
        printf("%d\t%s\n", i, bones[i]->GetName().c_str());
    }*/

    scene = nullptr;
    return std::move(model);
}

std::unique_ptr<CollisionModel> AssimpLoader::LoadCollision(const std::wstring& path)
{
    if (path.empty())
    {
        return nullptr;
    }

    directory_path = StringMethods::GetDirectoryPath(path);

    // インポート用のフラグを設定
    int flag = 0;
    flag |= aiProcess_Triangulate;
    flag |= aiProcess_PreTransformVertices;

    // ファイルを読み込む
    Assimp::Importer importer;
    auto scene = importer.ReadFile(StringMethods::GetString(path), flag);

    if (!scene)
    {
        // もし読み込みエラーが出たら表示する
        printf(importer.GetErrorString());
        printf("\n");
        return nullptr;
    }

    auto model = std::make_unique<CollisionModel>();
    auto& meshes = model->meshes;

    meshes.resize(scene->mNumMeshes);

    for (auto i = 0u; i < scene->mNumMeshes; i++)
    {
        const auto aimesh = scene->mMeshes[i];
        auto& mesh = model->meshes[i];

        // 頂点情報の読み込み
        std::vector<CollisionVertex> vertices(aimesh->mNumVertices);
        for (auto j = 0u; j < aimesh->mNumVertices; j++)
        {
            auto position = &(aimesh->mVertices[j]);
            auto normal = &(aimesh->mNormals[j]);

            auto& vertex = vertices[j];
            vertex.position = Vec3(position->x, position->y, position->z);
            vertex.normal = Vec3(normal->x, normal->y, normal->z);
        }

        // ポリゴン情報の読み込み
        mesh.polygons.resize(aimesh->mNumFaces);
        for (auto j = 0u; j < aimesh->mNumFaces; j++)
        {
            const auto& aiface = aimesh->mFaces[j];

            auto id0 = aiface.mIndices[0];
            auto id1 = aiface.mIndices[1];
            auto id2 = aiface.mIndices[2];

            auto p0 = vertices[id0].position;
            auto p1 = vertices[id1].position;
            auto p2 = vertices[id2].position;

            auto n0 = vertices[id0].normal;
            auto n1 = vertices[id1].normal;
            auto n2 = vertices[id2].normal;

            auto& polygon = mesh.polygons[j];
            polygon.vertices[0] = vertices[id0];
            polygon.vertices[1] = vertices[id1];
            polygon.vertices[2] = vertices[id2];
            polygon.normal = (n0 + n1 + n2).Normalized();
            
            // AABBを設定
            polygon.aabb.max = Vec3::Max(Vec3::Max(p0, p1), p2);
            polygon.aabb.min = Vec3::Min(Vec3::Min(p0, p1), p2);
        }

        // meshのAABBを設定
        if (!mesh.polygons.empty())
        {
            mesh.aabb = mesh.polygons[0].aabb;
        }
        for (auto j = 1; j < mesh.polygons.size(); j++)
        {
            mesh.aabb.max = Vec3::Max(mesh.aabb.max, mesh.polygons[j].aabb.max);
            mesh.aabb.min = Vec3::Min(mesh.aabb.min, mesh.polygons[j].aabb.min);
        }
    }

    // // modelのAABBを設定
    if (!meshes.empty())
    {
        model->aabb = meshes[0].aabb;
    }
    for (auto i = 1; i < meshes.size(); i++)
    {
        model->aabb.max = Vec3::Max(model->aabb.max, meshes[i].aabb.max);
        model->aabb.min = Vec3::Min(model->aabb.min, meshes[i].aabb.min);
    }

    scene = nullptr;
    return std::move(model);
}

void AssimpLoader::ProcessNode(Model& model, aiNode* node, const aiScene* scene)
{
    auto& meshes = model.meshes;
    auto& bones = model.bones; 

    for (auto i = 0u; i < node->mNumMeshes; i++)
    {
        auto mesh_index = node->mMeshes[i];
        auto aimesh = scene->mMeshes[mesh_index];

        LoadMesh(meshes[mesh_index], aimesh);
        LoadBones(bones, meshes[mesh_index], aimesh, node);
    }

    for (auto i = 0u; i < node->mNumChildren; i++)
    {
        ProcessNode(model, node->mChildren[i], scene);
    }
}

void AssimpLoader::LoadMesh(Mesh& dst, const aiMesh* src)
{
    auto pos_max = Vec3(FLT_MIN, FLT_MIN, FLT_MIN);
    auto pos_min = Vec3(FLT_MAX, FLT_MAX, FLT_MAX);
    
    aiVector3D zero3D(0.0f, 0.0f, 0.0f);
    aiVector3D zeroTangent(1.0f, 0.0f, 0.0f);
    aiColor4D zeroColor(0.0f, 0.0f, 0.0f, 0.0f);

    // 頂点データの読み込み
    dst.vertices.resize(src->mNumVertices);
    for (auto i = 0u; i < src->mNumVertices; ++i)
    {
        auto position = &(src->mVertices[i]);
        auto normal = &(src->mNormals[i]);
        auto uv = (src->HasTextureCoords(0)) ? &(src->mTextureCoords[0][i]) : &zero3D;
        auto tangent = (src->HasTangentsAndBitangents()) ? &(src->mTangents[i]) : &zeroTangent;
        auto color = (src->HasVertexColors(0)) ? &(src->mColors[0][i]) : &zeroColor;
        
        Vertex vertex = {};
        vertex.position = DirectX::XMFLOAT3(position->x, position->y, position->z);
        vertex.normal = DirectX::XMFLOAT3(normal->x, normal->y, normal->z);
        vertex.uv = DirectX::XMFLOAT2(uv->x, uv->y);
        vertex.tangent = DirectX::XMFLOAT3(tangent->x, tangent->y, tangent->z);
        vertex.color = DirectX::XMFLOAT4(color->r, color->g, color->b, color->a);

        dst.vertices[i] = vertex;

        pos_max.x = std::max(pos_max.x, position->x);
        pos_max.y = std::max(pos_max.y, position->y);
        pos_max.z = std::max(pos_max.z, position->z);

        pos_min.x = std::min(pos_min.x, position->x);
        pos_min.y = std::min(pos_min.y, position->y);
        pos_min.z = std::min(pos_min.z, position->z);
    }

    // AABBの設定
    dst.aabb = AABB{ pos_max, pos_min };
    
    // インデックスデータの読み込み
    dst.indices.resize(src->mNumFaces * 3);
    for (auto i = 0u; i < src->mNumFaces; ++i)
    {
        const auto& face = src->mFaces[i];

        int id0 = face.mIndices[0];
        int id1 = face.mIndices[1];
        int id2 = face.mIndices[2];

        dst.indices[i * 3 + 0] = id0;
        dst.indices[i * 3 + 1] = id1;
        dst.indices[i * 3 + 2] = id2;
    }

    dst.material_index = src->mMaterialIndex;
}

void AssimpLoader::LoadMaterial(Material& dst, const aiMaterial* src, const aiScene* scene)
{
    aiString path;

    float white[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    float pbr[] = { 0.0f, 0.8f, 0.0f, 1.0f };
    float normal[] = { 0.5f, 0.5f, 1.0f, 1.0f };

    std::unique_ptr<Texture2D> tex;

    // Albedoテクスチャの読み込み
    Texture2D::SetDefaultColor(white);
    if (src->GetTexture(aiTextureType_DIFFUSE, 0, &path) == aiReturn_SUCCESS)
    {
        tex = LoadTexture(path, src, scene);
    }
    else
    {
        tex = Texture2D::GetWhite();
    }
    dst.albedo_texture = std::move(tex);

    // Metallic, Roughness, Ambientテクスチャの読み込み
    Texture2D::SetDefaultColor(pbr);
    if (src->GetTexture(aiTextureType_DIFFUSE_ROUGHNESS, 0, &path) == aiReturn_SUCCESS)
    {
        tex = LoadTexture(path, src, scene);
    }
    else
    {
        tex = Texture2D::GetMono(pbr);
    }
    dst.pbr_texture = std::move(tex);

    // Normalテクスチャの読み込み
    Texture2D::SetDefaultColor(normal);
    if (src->GetTexture(aiTextureType_NORMALS, 0, &path) == aiReturn_SUCCESS)
    {
        tex = LoadTexture(path, src, scene);
    }
    else
    {
        tex = Texture2D::GetMono(normal);
    }
    dst.normal_texture = std::move(tex);

    // アルファモードを取得
    aiString alphaMode = aiString("OPAQUE");
    src->Get(AI_MATKEY_GLTF_ALPHAMODE, alphaMode);
    
    if (alphaMode == aiString("BLEND"))
    {
        dst.alpha_mode = 1;
    }
    else
    {
        dst.alpha_mode = 0;
    }

    // 色を取得
    aiColor4D color;
    src->Get(AI_MATKEY_COLOR_DIFFUSE, color);
    dst.base_color = { color.r, color.g, color.b, color.a };
    
    // プロパティ一覧の表示
    /*printf("%s\n", path.C_Str());
    auto prop = src->mProperties;
    int len = src->mNumProperties;
    for (int i = 0; i < len; i++)
    {
        printf("\t%s\n", prop[i]->mKey.C_Str());
    }*/
    
}

void AssimpLoader::LoadBones(BoneList& bones, Mesh& mesh, const aiMesh* pMesh, aiNode* node)
{
    for (auto i = 0u; i < pMesh->mNumBones; i++)
    {
        auto aibone = pMesh->mBones[i];
        auto name = std::string(aibone->mName.C_Str());
        
        Bone* bone = bones.Find(name);
        auto bone_index = bones.Index(bone);

        // オフセット行列を読み込む
        auto mat = aibone->mOffsetMatrix;
        auto ibmat = DirectX::XMMATRIX(
            mat.a1, mat.a2, mat.a3, mat.a4,
            mat.b1, mat.b2, mat.b3, mat.b4,
            mat.c1, mat.c2, mat.c3, mat.c4,
            mat.d1, mat.d2, mat.d3, mat.d4);
        bone->SetInvBindMatrix(ibmat);

        // ウェイトの値を読み込む
        auto weights = aibone->mWeights;
        for (auto j = 0u; j < aibone->mNumWeights; j++)
        {
            auto weight = weights[j].mWeight;
            auto vertexId = weights[j].mVertexId;
            auto vertex = &(mesh.vertices[vertexId]);

            auto n = vertex->bone_num;

            uint32_t indices[] = { 
                vertex->bone_indices.x, 
                vertex->bone_indices.y, 
                vertex->bone_indices.z, 
                vertex->bone_indices.w 
            };

            float weights[] = { 
                vertex->bone_weights.x, 
                vertex->bone_weights.y, 
                vertex->bone_weights.z, 
                vertex->bone_weights.w 
            };

            indices[n] = bone_index;
            weights[n] = weight;

            vertex->bone_indices = XMUINT4(indices);
            vertex->bone_weights = XMFLOAT4(weights);
            vertex->bone_num++;
        }
    }

    // ノードをボーンとして読み込む
    auto name = std::string(node->mName.C_Str());
    
    Bone* bone = bones.Find(name);
    int bone_index = bones.Index(bone);

    for (auto i = 0u; i < pMesh->mNumVertices; ++i)
    {
        auto vertex = &(mesh.vertices[i]);
        auto n = vertex->bone_num;

        if (n >= 4) continue;

        uint32_t indices[] = {
            vertex->bone_indices.x,
            vertex->bone_indices.y, 
            vertex->bone_indices.z, 
            vertex->bone_indices.w
        };

        float weights[] = { 
            vertex->bone_weights.x, 
            vertex->bone_weights.y, 
            vertex->bone_weights.z, 
            vertex->bone_weights.w
        };

        indices[n] = bone_index;
        weights[n] = 1.0f - (weights[0] + weights[1] + weights[2] + weights[3]);

        vertex->bone_indices = XMUINT4(indices);
        vertex->bone_weights = XMFLOAT4(weights);
        vertex->bone_num++;
    }
}

std::unique_ptr<Texture2D> AssimpLoader::LoadTexture(aiString path, const aiMaterial* src, const aiScene* scene)
{
    // 埋め込み画像かどうか
    const aiTexture* embeddedTexture = scene->GetEmbeddedTexture(path.C_Str());
    if (embeddedTexture != nullptr)
    {
        return LoadEmbeddedTexture(embeddedTexture);
    }
    else
    {
        // テクスチャパスは相対パスで入っているので、ディレクトリのパスとくっつける
        auto file = std::string(path.C_Str());
        auto texturePath = directory_path + StringMethods::GetWideString(file);

        return Texture2D::Load(texturePath);
    }
}

std::unique_ptr<Texture2D> AssimpLoader::LoadEmbeddedTexture(const aiTexture* texture)
{
    if (texture->mHeight == 0)
    {
        return Texture2D::Load(texture->pcData, texture->mWidth);
    }
    else
    {
        return Texture2D::Load(texture->pcData, static_cast<size_t>(texture->mWidth) * texture->mHeight);
    }
}

void AssimpLoader::CreateBoneTree(BoneList& bone_tree, Bone* parent, const aiNode* node)
{
    auto name = std::string(node->mName.C_Str());
    auto mat = node->mTransformation;
    
    // 行列から scale, rotation, position を取得
    aiVector3D scale;
    aiQuaternion rot;
    aiVector3D pos;
    mat.Decompose(scale, rot, pos);

    // 新しいボーンを生成
    auto bone = new Bone(name);
    bone->SetGlobalMatrix(
        DirectX::XMMATRIX(
            mat.a1, mat.a2, mat.a3, mat.a4,
            mat.b1, mat.b2, mat.b3, mat.b4,
            mat.c1, mat.c2, mat.c3, mat.c4,
            mat.d1, mat.d2, mat.d3, mat.d4)
    );
    bone->SetScale(Vec3(scale.x, scale.y, scale.z));
    bone->SetRotation(Quaternion(rot.x, rot.y, rot.z, rot.w));
    bone->SetPosition(Vec3(pos.x, pos.y, pos.z));
    bone->SetParent(parent);

    bone_tree.Append(bone);

    // 子ノードを探索
    for (auto i = 0u; i < node->mNumChildren; i++)
    {
        CreateBoneTree(bone_tree, bone, node->mChildren[i]);
    }
}

void AssimpLoader::LoadAnimation(Animation& animation, aiAnimation* src, const BoneList& bones)
{
    auto anim_name = std::string(src->mName.C_Str());
    printf("Animation load: %s\n", anim_name.c_str());

    animation.SetName(anim_name);
    animation.SetDuration((float)src->mDuration);
    animation.SetTicksPerSecond((float)src->mTicksPerSecond);

    for (auto i = 0u; i < src->mNumChannels; i++)
    {
        auto channel = src->mChannels[i];
        auto name = std::string(channel->mNodeName.C_Str());
        auto bone = bones.Find(name);

        std::vector<Animation::Vec3Key> scaling_keys;
        std::vector<Animation::QuatKey> rotation_keys;
        std::vector<Animation::Vec3Key> position_keys;

        // 拡大縮小キーの読み込み
        scaling_keys.resize(channel->mNumScalingKeys);
        for (int j = 0; j < scaling_keys.size(); j++)
        {
            auto time = (float)channel->mScalingKeys[j].mTime;
            auto value = channel->mScalingKeys[j].mValue;
            
            scaling_keys[j].time = time;
            scaling_keys[j].value = Vec3(value.x, value.y, value.z);
            scaling_keys[j].easing = Easing::Linear;
        }

        // 回転キーの読み込み
        rotation_keys.resize(channel->mNumRotationKeys);
        for (int j = 0; j < rotation_keys.size(); j++)
        {
            auto time = (float)channel->mRotationKeys[j].mTime;
            auto value = channel->mRotationKeys[j].mValue;

            rotation_keys[j].time = time;
            rotation_keys[j].value = Quaternion(value.x, value.y, value.z, value.w);
            rotation_keys[j].easing = Easing::Linear;
        }
        
        // 移動キーの読み込み
        position_keys.resize(channel->mNumPositionKeys);
        for (int j = 0; j < position_keys.size(); j++)
        {
            auto time = (float)channel->mPositionKeys[j].mTime;
            auto value = channel->mPositionKeys[j].mValue;

            position_keys[j].time = time;
            position_keys[j].value = Vec3(value.x, value.y, value.z);
            position_keys[j].easing = Easing::Linear;
        }

        // チャンネルを生成して追加
        auto bone_channel = new Animation::BoneChannel{};
        bone_channel->name = name;
        bone_channel->scaling_keys = scaling_keys;
        bone_channel->rotation_keys = rotation_keys;
        bone_channel->position_keys = position_keys;
        bone_channel->type = Animation::TYPE_BONE;

        animation.AddChannel(bone_channel);
    }

}

void AssimpLoader::GenSmoothNormal(Mesh& dst)
{
    float distance = 1e-8f;

    for (int i = 0; i < dst.vertices.size(); i++)
    {
        auto position1 = (Vec3)dst.vertices[i].position;
        auto normal = Vec3(0, 0, 0);

        for (int j = 0; j < dst.vertices.size(); j++)
        {
            auto position2 = (Vec3)dst.vertices[j].position;
            auto v = position1 - position2;

            if (v.Length() < distance)
            {
                normal = normal + dst.vertices[j].normal;
            }
        }

        normal = normal.Normalized();

        auto vertex = &(dst.vertices[i]);
        vertex->smooth_normal = normal;
    }
}


