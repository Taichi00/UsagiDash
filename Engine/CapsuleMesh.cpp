#include "CapsuleMesh.h"
#include "SharedStruct.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Texture2D.h"
#include <DirectXMath.h>
#include <vector>

#define PI 3.14159265359

using namespace DirectX;

Model CapsuleMesh::Load(float radius, float height, float r, float g, float b)
{
    int sliceNum = 40;
    int stackNum = 20;
    float height2 = height / 2;

    std::vector<Vertex> vertices;

    auto v0 = Vertex{};
    v0.Position = Vec3(0, 1, 0) * radius + Vec3(0, height2, 0);
    v0.Normal = XMFLOAT3(0, 1, 0);
    v0.SmoothNormal = v0.Normal;
    vertices.push_back(v0);

    for (int i = 0; i < stackNum / 2; i++)
    {
        auto phi = PI * double(i + 1) / double(stackNum);
        for (int j = 0; j < sliceNum; j++)
        {
            auto theta = 2.0 * PI * double(j) / double(sliceNum);
            auto x = sin(phi) * cos(theta);
            auto y = cos(phi);
            auto z = sin(phi) * sin(theta);

            auto v = Vertex{};
            v.Position = Vec3(x, y, z) * radius + Vec3(0, height2, 0);
            v.Normal = Vec3(x, y, z).normalized();
            v.SmoothNormal = v.Normal;
            vertices.push_back(v);
        }
    }

    for (int i = 0; i < stackNum; i++)
    {
        auto phi = PI * double(i + stackNum / 2) / double(stackNum);
        for (int j = 0; j < sliceNum; j++)
        {
            auto theta = 2.0 * PI * double(j) / double(sliceNum);
            auto x = sin(phi) * cos(theta);
            auto y = cos(phi);
            auto z = sin(phi) * sin(theta);

            auto v = Vertex{};
            v.Position = Vec3(x, y, z) * radius - Vec3(0, height2, 0);
            v.Normal = Vec3(x, y, z).normalized();
            v.SmoothNormal = v.Normal;
            vertices.push_back(v);
        }
    }

    auto v1 = Vertex{};
    v1.Position = Vec3(0, -1, 0) * radius - Vec3(0, height2, 0);
    v1.Normal = XMFLOAT3(0, -1, 0);
    v1.SmoothNormal = v1.Normal;
    vertices.push_back(v1);

    std::vector<uint32_t> indices;

    for (int i = 0; i < sliceNum; ++i)
    {
        auto i0 = i + 1;
        auto i1 = (i + 1) % sliceNum + 1;
        indices.push_back(0);
        indices.push_back(i1);
        indices.push_back(i0);

        i0 = i + sliceNum * (stackNum - 2) + 1;
        i1 = (i + 1) % sliceNum + sliceNum * (stackNum - 2) + 1;
        indices.push_back(vertices.size() - 1);
        indices.push_back(i0);
        indices.push_back(i1);
    }

    for (int j = 0; j < stackNum - 2; j++)
    {
        auto j0 = j * sliceNum + 1;
        auto j1 = (j + 1) * sliceNum + 1;
        for (int i = 0; i < sliceNum; i++)
        {
            auto i0 = j0 + i;
            auto i1 = j0 + (i + 1) % sliceNum;
            auto i2 = j1 + (i + 1) % sliceNum;
            auto i3 = j1 + i;
            indices.push_back(i0);
            indices.push_back(i1);
            indices.push_back(i2);
            indices.push_back(i2);
            indices.push_back(i3);
            indices.push_back(i0);
        }
    }

    auto mesh = Mesh{};
    mesh.Vertices = vertices;
    mesh.Indices = indices;
    mesh.MaterialIndex = 0;

    std::vector<Mesh> meshes;
    meshes.push_back(mesh);

    for (int i = 0; i < meshes.size(); i++)
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

    float pbr[4] = { 1, 0.1, 0, 1 };

    auto material = Material{};
    material.BaseColor = { r, g, b, 1 };
    material.Texture = Texture2D::GetWhite();
    material.PbrTexture = Texture2D::GetMono(pbr);
    material.Shininess = 10;

    std::vector<Material> materials;
    materials.push_back(material);

    auto model = Model{};
    model.Meshes = meshes;
    model.Materials = materials;

    return model;
}
