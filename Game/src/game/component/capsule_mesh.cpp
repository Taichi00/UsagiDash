#include "game/component/capsule_mesh.h"
#include "engine/engine.h"
#include "engine/shared_struct.h"
#include "engine/vertex_buffer.h"
#include "engine/index_buffer.h"
#include "game/resource/texture2d.h"
#include "game/game.h"
#include "game/resource/model.h"
#include <DirectXMath.h>
#include <vector>

#define PI 3.14159265359

using namespace DirectX;

std::unique_ptr<Model> CapsuleMesh::Load(float radius, float height, float r, float g, float b)
{
    int sliceNum = 40;
    int stackNum = 20;
    float height2 = height / 2;

    auto model = std::make_unique<Model>();

    std::vector<Vertex> vertices;

    auto v0 = Vertex{};
    v0.position = Vec3(0, 1, 0) * radius + Vec3(0, height2, 0);
    v0.normal = XMFLOAT3(0, 1, 0);
    v0.tangent = XMFLOAT3(1, 0, 0);
    v0.smooth_normal = v0.normal;
    vertices.push_back(v0);

    for (int i = 0; i < stackNum / 2; i++)
    {
        auto phi = PI * double(i + 1) / double(stackNum);
        for (int j = 0; j < sliceNum; j++)
        {
            auto theta = 2.0 * PI * double(j) / double(sliceNum);
            float x = (float)(sin(phi) * cos(theta));
            float y = (float)cos(phi);
            float z = (float)(sin(phi) * sin(theta));

            auto v = Vertex{};
            v.position = Vec3(x, y, z) * radius + Vec3(0, height2, 0);
            v.normal = Vec3(x, y, z).Normalized();
            v.tangent = XMFLOAT3(1, 0, 0);
            v.smooth_normal = v.normal;
            vertices.push_back(v);
        }
    }

    for (int i = 0; i < stackNum; i++)
    {
        auto phi = PI * double(i + stackNum / 2) / double(stackNum);
        for (int j = 0; j < sliceNum; j++)
        {
            auto theta = 2.0 * PI * double(j) / double(sliceNum);
            float x = (float)(sin(phi) * cos(theta));
            float y = (float)cos(phi);
            float z = (float)(sin(phi) * sin(theta));

            auto v = Vertex{};
            v.position = Vec3(x, y, z) * radius - Vec3(0, height2, 0);
            v.normal = Vec3(x, y, z).Normalized();
            v.tangent = XMFLOAT3(1, 0, 0);
            v.smooth_normal = v.normal;
            vertices.push_back(v);
        }
    }

    auto v1 = Vertex{};
    v1.position = Vec3(0, -1, 0) * radius - Vec3(0, height2, 0);
    v1.normal = XMFLOAT3(0, -1, 0);
    v1.tangent = XMFLOAT3(1, 0, 0);
    v1.smooth_normal = v1.normal;
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
        indices.push_back((int)vertices.size() - 1);
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
    mesh.vertices = vertices;
    mesh.indices = indices;
    mesh.material_index = 0;

    auto& meshes = model->meshes;
    meshes.push_back(mesh);

    for (int i = 0; i < meshes.size(); i++)
    {
        // 頂点バッファの生成
        auto vSize = sizeof(Vertex) * meshes[i].vertices.size();
        auto stride = sizeof(Vertex);
        auto vertices = meshes[i].vertices.data();
        auto pVB = std::make_unique<VertexBuffer>(vSize, stride, vertices);
        if (!pVB->IsValid())
        {
            printf("頂点バッファの生成に失敗\n");
            break;
        }
        meshes[i].vertex_buffer = std::move(pVB);

        // インデックスバッファの生成
        auto iSize = sizeof(uint32_t) * meshes[i].indices.size();
        auto indices = meshes[i].indices.data();
        auto pIB = std::make_unique<IndexBuffer>(iSize, indices);
        if (!pIB->IsValid())
        {
            printf("インデックスバッファの生成に失敗\n");
            break;
        }
        meshes[i].index_buffer = std::move(pIB);
    }

    float pbr[4] = { 0.f, 0.1f, 0.f, 1.f };
    float normal[4] = { 0.5f, 0.5f, 1.f, 1.f };

    auto material = Material{};
    material.base_color = { r, g, b, 1 };
    material.albedo_texture = Texture2D::GetWhite();
    material.pbr_texture = Texture2D::GetMono(pbr);
    material.normal_texture = Texture2D::GetMono(normal);

    auto& materials = model->materials;
    materials.push_back(material);

    // 頂点バッファ・インデックスバッファの生成
    for (size_t i = 0; i < meshes.size(); i++)
    {
        meshes[i].vertex_buffer = Game::Get()->GetEngine()->CreateVertexBuffer(meshes[i].vertices);
        meshes[i].index_buffer = Game::Get()->GetEngine()->CreateIndexBuffer(meshes[i].indices);
    }

    // ディスクリプタヒープの生成
    model->descriptor_heap = Game::Get()->GetEngine()->CreateDescriptorHeap((int)materials.size() * 3);

    // ディスクリプタハンドルの取得、SRVの生成
    for (auto& mat : materials)
    {
        mat.albedo_handle = model->descriptor_heap->Alloc();
        mat.pbr_handle = model->descriptor_heap->Alloc();
        mat.normal_handle = model->descriptor_heap->Alloc();

        Game::Get()->GetEngine()->CreateShaderResourceView2D(*mat.albedo_texture, mat.albedo_handle);
        Game::Get()->GetEngine()->CreateShaderResourceView2D(*mat.pbr_texture, mat.pbr_handle);
        Game::Get()->GetEngine()->CreateShaderResourceView2D(*mat.normal_texture, mat.normal_handle);
    }

    return std::move(model);
}
