#include "AssimpLoader.h"
#include "SharedStruct.h"
#include "Texture2D.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "PipelineState.h"
#include "Bone.h"
#include "BoneList.h"
#include "Animation.h"
#include "Vec.h"
#include "Quaternion.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/material.h>
#include <assimp/pbrmaterial.h>
#include "d3dx12.h"
#include <filesystem>
#include <d3d12.h>
#include <stdio.h>
#include <DirectXMath.h>

namespace fs = std::filesystem;

std::wstring directoryPath;

std::wstring GetDirectoryPath(const std::wstring& origin)
{
    fs::path p = origin.c_str();
    return p.remove_filename().c_str();
}

std::string ToUTF8(const std::wstring& value)
{
    auto length = WideCharToMultiByte(CP_UTF8, 0U, value.data(), -1, nullptr, 0, nullptr, nullptr);
    auto buffer = new char[length];

    WideCharToMultiByte(CP_UTF8, 0U, value.data(), -1, buffer, length, nullptr, nullptr);

    std::string result(buffer);
    delete[] buffer;
    buffer = nullptr;

    return result;
}

// std::string（マルチバイト文字列）からstd::wstring（ワイド文字列）を得る
std::wstring ToWideString(const std::string& str)
{
    auto num1 = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED | MB_ERR_INVALID_CHARS, str.c_str(), -1, nullptr, 0);

    if (num1 == 0)
    {
        return L"";
    }

    std::wstring wstr;
    wstr.resize(num1 - 1);  // ヌル文字をつけない

    auto num2 = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED | MB_ERR_INVALID_CHARS, str.c_str(), -1, &wstr[0], num1);

    assert(num1 == num2);
    return wstr;
}

AssimpLoader::AssimpLoader()
{

}

bool AssimpLoader::Load(const wchar_t* filename, Model& model, std::vector<Animation*>& animations)
{
    if (filename == nullptr)
    {
        return false;
    }

    directoryPath = GetDirectoryPath(filename);

    auto& meshes = model.Meshes;
    auto& materials = model.Materials;
    auto& bones = model.Bones;

    auto path = ToUTF8(filename);

    Assimp::Importer importer;
    int flag = 0;
    flag |= aiProcess_Triangulate;
    flag |= aiProcess_LimitBoneWeights;
    flag |= aiProcess_FlipUVs;

    auto pScene = importer.ReadFile(path, flag);
    
    if (pScene == nullptr)
    {
        // もし読み込みエラーが出たら表示する
        printf(importer.GetErrorString());
        printf("\n");
        return false;
    }
    
    meshes.clear();
    meshes.resize(pScene->mNumMeshes);
    bones.Clear();

    // Boneの階層構造を構築
    BuildBoneHierarchy(bones, pScene->mRootNode, nullptr);

    // Mesh, Boneの読み込み
    ProcessNode(model, pScene->mRootNode, pScene);

    // Materialの読み込み
    materials.clear();
    materials.resize(pScene->mNumMaterials);
    for (size_t i = 0; i < materials.size(); ++i)
    {
        const auto pMaterial = pScene->mMaterials[i];
        LoadMaterial(materials[i], pMaterial, pScene);
    }

    // Aniamtionの読み込み
    animations.clear();
    animations.resize(pScene->mNumAnimations);
    for (size_t i = 0; i < animations.size(); i++)
    {
        animations[i] = new Animation();
        LoadAnimation(animations[i], pScene->mAnimations[i], &bones);
    }

    // SmoothNormalの生成
    flag |= aiProcess_GenSmoothNormals;
    flag |= aiProcess_ForceGenNormals;
    pScene = importer.ReadFile(path, flag);

    for (size_t i = 0; i < meshes.size(); ++i)
    {
        auto mesh = &(meshes[i]);
        auto aimesh = pScene->mMeshes[i];
        for (size_t j = 0; j < meshes[i].Vertices.size(); ++j)
        {
            auto vertex = &(meshes[i].Vertices[j]);
            auto normal = &(aimesh->mNormals[j]);
            vertex->SmoothNormal = DirectX::XMFLOAT3(normal->x, normal->y, normal->z);
        }
    }

    
    for (size_t i = 0; i < meshes.size(); i++)
    {
        // 頂点バッファの生成
        auto vSize = sizeof(Vertex) * meshes[i].Vertices.size();
        auto stride = sizeof(Vertex);
        auto vertices = meshes[i].Vertices.data();
        auto pVB = new VertexBuffer(vSize, stride, vertices);
        if (!pVB->IsValid())
        {
            printf("頂点バッファの生成に失敗\n");
            break;
        }
        meshes[i].pVertexBuffer = pVB;

        // インデックスバッファの生成
        auto iSize = sizeof(uint32_t) * meshes[i].Indices.size();
        auto indices = meshes[i].Indices.data();
        auto pIB = new IndexBuffer(iSize, indices);
        if (!pIB->IsValid())
        {
            printf("インデックスバッファの生成に失敗\n");
            break;
        }
        meshes[i].pIndexBuffer = pIB;
    }

    //for (int i = 0; i < bones.Size(); i++)
    //{
    //    printf("%d\t%s\n", i, bones[i]->GetName().c_str());
    //}

    pScene = nullptr;
    return true;
}

void AssimpLoader::ProcessNode(Model& model, aiNode* node, const aiScene* scene)
{
    auto& meshes = model.Meshes;
    auto& bones = model.Bones;

    for (UINT i = 0; i < node->mNumMeshes; i++)
    {
        auto meshIndex = node->mMeshes[i];
        aiMesh* pMesh = scene->mMeshes[meshIndex];
        LoadMesh(meshes[meshIndex], pMesh);
        
        LoadBones(bones, meshes[meshIndex], pMesh);

        // アウトライン用のスムーズな法線を生成
        //GenSmoothNormal(meshes[meshIndex]);
    }

    for (UINT i = 0; i < node->mNumChildren; i++)
    {
        ProcessNode(model, node->mChildren[i], scene);
    }
}

void AssimpLoader::LoadMesh(Mesh& dst, const aiMesh* src)
{
    aiVector3D zero3D(0.0f, 0.0f, 0.0f);
    aiColor4D zeroColor(0.0f, 0.0f, 0.0f, 0.0f);

    dst.Vertices.resize(src->mNumVertices);
    
    for (auto i = 0u; i < src->mNumVertices; ++i)
    {
        auto position = &(src->mVertices[i]);
        auto normal = &(src->mNormals[i]);
        auto uv = (src->HasTextureCoords(0)) ? &(src->mTextureCoords[0][i]) : &zero3D;
        auto tangent = (src->HasTangentsAndBitangents()) ? &(src->mTangents[i]) : &zero3D;
        auto color = (src->HasVertexColors(0)) ? &(src->mColors[0][i]) : &zeroColor;
        
        Vertex vertex = {};
        vertex.Position = DirectX::XMFLOAT3(position->x, position->y, position->z);
        vertex.Normal = DirectX::XMFLOAT3(normal->x, normal->y, normal->z);
        vertex.UV = DirectX::XMFLOAT2(uv->x, uv->y);
        vertex.Tangent = DirectX::XMFLOAT3(tangent->x, tangent->y, tangent->z);
        vertex.Color = DirectX::XMFLOAT4(color->r, color->g, color->b, color->a);

        dst.Vertices[i] = vertex;
    }

    dst.Indices.resize(src->mNumFaces * 3);
    
    for (auto i = 0u; i < src->mNumFaces; ++i)
    {
        const auto& face = src->mFaces[i];

        int id0 = face.mIndices[0];
        int id1 = face.mIndices[1];
        int id2 = face.mIndices[2];

        dst.Indices[i * 3 + 0] = id0;
        dst.Indices[i * 3 + 1] = id1;
        dst.Indices[i * 3 + 2] = id2;
    }

    dst.MaterialIndex = src->mMaterialIndex;
}

void AssimpLoader::LoadMaterial(Material& dst, const aiMaterial* src, const aiScene* scene)
{
    aiString path;
    src->GetTexture(aiTextureType_DIFFUSE, 0, &path);

    const aiTexture* embeddedTexture = scene->GetEmbeddedTexture(path.C_Str());
    if (embeddedTexture != nullptr)
    {
        LoadEmbeddedTexture(dst, embeddedTexture);
    }
    else
    {
        LoadTexture(path, dst, src);
    }

    // アルファモードを取得
    aiString alphaMode = aiString("OPAQUE");
    src->Get(AI_MATKEY_GLTF_ALPHAMODE, alphaMode);
    
    if (alphaMode == aiString("BLEND"))
    {
        dst.AlphaMode = 1;

    }
    else
    {
        dst.AlphaMode = 0;
    }

    // 色を取得
    aiColor4D color;
    src->Get(AI_MATKEY_COLOR_DIFFUSE, color);
    dst.BaseColor = { color.r, color.g, color.b, color.a };
    
    // 光沢度を取得
    float shininess;
    src->Get(AI_MATKEY_SHININESS, shininess);
    dst.Shininess = shininess;

    // プロパティ一覧の表示
    /*printf("%s\n", path.C_Str());
    auto prop = src->mProperties;
    int len = src->mNumProperties;
    for (int i = 0; i < len; i++)
    {
        printf("\t%s\n", prop[i]->mKey.C_Str());
    }*/
    
}

void AssimpLoader::LoadBones(BoneList& bones, Mesh& mesh, const aiMesh* pMesh)
{
    for (auto i = 0; i < pMesh->mNumBones; i++)
    {
        auto pBone = pMesh->mBones[i];
        auto name = std::string(pBone->mName.C_Str());
        
        Bone* bone = bones.Find(name);
        int boneIndex = bones.Index(bone);

        auto mat = pBone->mOffsetMatrix;
        auto ibmat = DirectX::XMMATRIX(
            mat.a1, mat.a2, mat.a3, mat.a4,
            mat.b1, mat.b2, mat.b3, mat.b4,
            mat.c1, mat.c2, mat.c3, mat.c4,
            mat.d1, mat.d2, mat.d3, mat.d4);
        bone->SetInvBindMatrix(ibmat);

        auto weights = pBone->mWeights;
        for (auto j = 0; j < pBone->mNumWeights; j++)
        {
            auto weight = weights[j].mWeight;
            auto vertexId = weights[j].mVertexId;
            auto vertex = &(mesh.Vertices[vertexId]);

            auto n = vertex->BoneNum;
            uint32_t indices[] = { vertex->BoneIndices.x, vertex->BoneIndices.y, vertex->BoneIndices.z, vertex->BoneIndices.w };
            float weights[] = { vertex->BoneWeights.x, vertex->BoneWeights.y, vertex->BoneWeights.z, vertex->BoneWeights.w };

            indices[n] = boneIndex;
            weights[n] = weight;

            vertex->BoneIndices = XMUINT4(indices);
            vertex->BoneWeights = XMFLOAT4(weights);
            vertex->BoneNum++;
        }
    }
}

void AssimpLoader::LoadTexture(aiString path, Material& dst, const aiMaterial* src)
{
    // テクスチャパスは相対パスで入っているので、ファイルの場所とくっつける
    auto file = std::string(path.C_Str());
    auto texturePath = directoryPath + ToWideString(file);
    
    //dst.DiffuseMap = dir + ToWideString(file);
    dst.Texture = Texture2D::Get(texturePath);
}

void AssimpLoader::LoadEmbeddedTexture(Material& dst, const aiTexture* texture)
{
    if (texture->mHeight == 0)
    {
        dst.Texture = Texture2D::Get(texture->pcData, texture->mWidth);
    }
    else
    {
        dst.Texture = Texture2D::Get(texture->pcData, texture->mWidth * texture->mHeight);
    }
}

void AssimpLoader::BuildBoneHierarchy(BoneList& bones, aiNode* node, Bone* parentBone)
{
    auto name = std::string(node->mName.C_Str());
    auto mat = node->mTransformation;
    
    Bone* bone = new Bone(name);
    bone->SetGlobalMatrix(
        DirectX::XMMATRIX(
            mat.a1, mat.a2, mat.a3, mat.a4,
            mat.b1, mat.b2, mat.b3, mat.b4,
            mat.c1, mat.c2, mat.c3, mat.c4,
            mat.d1, mat.d2, mat.d3, mat.d4));

    aiVector3D scale;
    aiQuaternion rot;
    aiVector3D pos;
    mat.Decompose(scale, rot, pos);

    bone->SetScale(Vec3(scale.x, scale.y, scale.z));
    bone->SetRotation(Quaternion(rot.x, rot.y, rot.z, rot.w));
    bone->SetPosition(Vec3(pos.x, pos.y, pos.z));

    if (parentBone != nullptr)
    {
        bone->SetParent(parentBone);
        parentBone->AddChild(bone);
    }
    bones.Append(bone);

    for (UINT i = 0; i < node->mNumChildren; i++)
    {
        BuildBoneHierarchy(bones, node->mChildren[i], bone);
    }
}

void AssimpLoader::LoadAnimation(Animation* animation, aiAnimation* pAnimation, BoneList* bones)
{
    auto animName = std::string(pAnimation->mName.C_Str());
    printf("Animation load: %s\n", animName.c_str());

    animation->SetName(animName);
    animation->SetDuration(pAnimation->mDuration);
    animation->SetTicksPerSecond(pAnimation->mTicksPerSecond);

    for (int i = 0; i < pAnimation->mNumChannels; i++)
    {
        auto channel = pAnimation->mChannels[i];
        auto name = std::string(channel->mNodeName.C_Str());
        auto bone = bones->Find(name);
        auto mtxGlobal = XMMatrixInverse(nullptr, bone->GetGlobalMatrix());

        std::vector<VectorKey> scalingKeys;
        std::vector<QuatKey> rotationKeys;
        std::vector<VectorKey> positionKeys;

        scalingKeys.resize(channel->mNumScalingKeys);
        for (int j = 0; j < scalingKeys.size(); j++)
        {
            auto time = channel->mScalingKeys[j].mTime;
            auto value = channel->mScalingKeys[j].mValue;
            auto scale = Vec3(value.x, value.y, value.z);
            scalingKeys[j] = { time, scale };
        }

        rotationKeys.resize(channel->mNumRotationKeys);
        for (int j = 0; j < rotationKeys.size(); j++)
        {
            auto time = channel->mRotationKeys[j].mTime;
            auto value = channel->mRotationKeys[j].mValue;
            auto rot = Quaternion(value.x, value.y, value.z, value.w);
            rotationKeys[j] = { time, rot };
        }
        
        positionKeys.resize(channel->mNumPositionKeys);
        for (int j = 0; j < positionKeys.size(); j++)
        {
            auto time = channel->mPositionKeys[j].mTime;
            auto value = channel->mPositionKeys[j].mValue;
            /*auto xmf = XMFLOAT3(value.x, value.y, value.z);
            auto mtx = XMMatrixTranslationFromVector(XMLoadFloat3(&xmf)) * mtxGlobal;*/
            auto pos = Vec3(value.x, value.y, value.z);
            positionKeys[j] = { time, pos };
        }

        Channel ch = { name, scalingKeys, rotationKeys, positionKeys };
        animation->AddChannel(ch);
    }

}

void AssimpLoader::GenSmoothNormal(Mesh& dst)
{
    float distance = 1e-8f;

    for (int i = 0; i < dst.Vertices.size(); i++)
    {
        auto position1 = (Vec3)dst.Vertices[i].Position;
        auto normal = Vec3(0, 0, 0);

        for (int j = 0; j < dst.Vertices.size(); j++)
        {
            auto position2 = (Vec3)dst.Vertices[j].Position;
            auto v = position1 - position2;

            if (v.length() < distance)
            {
                normal = normal + dst.Vertices[j].Normal;
            }
        }

        normal = normal.normalized();

        auto vertex = &(dst.Vertices[i]);
        vertex->SmoothNormal = normal;
    }
}


