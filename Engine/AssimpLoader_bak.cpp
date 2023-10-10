#include "AssimpLoader.h"
#include "SharedStruct.h"
#include "Texture2D.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "PipelineState.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/material.h>
#include <assimp/pbrmaterial.h>
#include "d3dx12.h"
#include <filesystem>
#include <d3d12.h>
#include <stdio.h>

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

bool AssimpLoader::Load(ImportSettings settings)
{
    if (settings.filename == nullptr)
    {
        return false;
    }

    directoryPath = GetDirectoryPath(settings.filename);

    auto& meshes = settings.meshes;
    auto& materials = settings.materials;
    auto inverseU = settings.inverseU;
    auto inverseV = settings.inverseV;

    auto path = ToUTF8(settings.filename);

    Assimp::Importer importer;
    int flag = 0;
    flag |= aiProcess_Triangulate;
    //flag |= aiProcess_PreTransformVertices;
    //flag |= aiProcess_CalcTangentSpace;
    //flag |= aiProcess_GenSmoothNormals;
    //flag |= aiProcess_GenUVCoords;
    //flag |= aiProcess_RemoveRedundantMaterials;
    //flag |= aiProcess_OptimizeMeshes;

    auto pScene = importer.ReadFile(path, flag);

    if (pScene == nullptr)
    {
        // もし読み込みエラーが出たら表示する
        printf(importer.GetErrorString());
        printf("\n");
        return false;
    }

    // Meshの読み込み
    meshes.clear();
    meshes.resize(pScene->mNumMeshes);
    for (size_t i = 0; i < meshes.size(); ++i)
    {
        const auto pMesh = pScene->mMeshes[i];
        LoadMesh(meshes[i], pMesh, inverseU, inverseV);
        LoadBones(meshes[i], pMesh);

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

    // Materialの読み込み
    materials.clear();
    materials.resize(pScene->mNumMaterials);
    for (size_t i = 0; i < materials.size(); ++i)
    {
        const auto pMaterial = pScene->mMaterials[i];
        LoadMaterial(materials[i], pMaterial, pScene);
    }

    pScene = nullptr;
    return true;
}

void AssimpLoader::LoadMesh(Mesh& dst, const aiMesh* src, bool inverseU, bool inverseV)
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

        // 反転オプションがあったらUVを反転させる
        if (inverseU)
        {
            uv->x = 1 - uv->x;
        }
        if (inverseV)
        {
            uv->y = 1 - uv->y;
        }

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

    /*printf("%s\n", path.C_Str());
    auto prop = src->mProperties;
    int len = src->mNumProperties;
    for (int i = 0; i < len; i++)
    {
        printf("\t%s\n", prop[i]->mKey.C_Str());
    }*/

}

void AssimpLoader::LoadBones(Mesh& dst, const aiMesh* src)
{
    printf("%d\n", src->mNumBones);
    for (auto i = 0; i < src->mNumBones; i++)
    {
        auto bone = src->mBones[i];

        auto name = bone->mName;
        auto weights = bone->mWeights;

        printf("%s\n", name.C_Str());

        for (auto j = 0; j < bone->mNumWeights; j++)
        {
            auto weight = weights[j].mWeight;
            auto vertexId = weights[j].mWeight;

            //printf("\t%d: %f\n", vertexId, weight);
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

