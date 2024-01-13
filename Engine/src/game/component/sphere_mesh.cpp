#include "game/component/sphere_mesh.h"
#include "engine/engine.h"
#include "engine/shared_struct.h"
#include "engine/vertex_buffer.h"
#include "engine/index_buffer.h"
#include "engine/texture2d.h"
#include "game/game.h"
#include "game/model.h"
#include <DirectXMath.h>
#include <vector>

#define PI 3.14159265359

using namespace DirectX;

std::unique_ptr<Model> SphereMesh::Load(float radius, float r, float g, float b)
{
    int sliceNum = 40;
    int stackNum = 20;

    auto model = std::make_unique<Model>();

    std::vector<Vertex> vertices;

    auto v0 = Vertex{};
    v0.position = Vec3(0, 1, 0) * radius;
    v0.normal = XMFLOAT3(0, 1, 0);
    v0.tangent = Vec3(1, 0, 0);
    v0.smooth_normal = v0.normal;
    vertices.push_back(v0);

    for (int i = 0; i < stackNum - 1; i++)
    {
        auto phi = PI * double(i + 1) / double(stackNum);
        for (int j = 0; j < sliceNum; j++)
        {
            auto theta = 2.0 * PI * double(j) / double(sliceNum);
            auto x = sin(phi) * cos(theta);
            auto y = cos(phi);
            auto z = sin(phi) * sin(theta);

            auto v = Vertex{};
            v.position = Vec3(x, y, z) * radius;
            v.normal = Vec3(x, y, z).Normalized();
            v.tangent = Vec3(1, 0, 0);
            v.smooth_normal = v.normal;
            vertices.push_back(v);
        }
    }

    auto v1 = Vertex{};
    v1.position = Vec3(0, -1, 0) * radius;
    v1.normal = XMFLOAT3(0, -1, 0);
    v1.tangent = Vec3(1, 0, 0);
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
    mesh.vertices = vertices;
    mesh.indices = indices;
    mesh.material_index = 0;

    auto& meshes = model->meshes;
    meshes.push_back(mesh);

    float pbr[4] = { 0, 0.8, 0, 1 };
    float normal[] = { 0.5, 0.5, 1, 1 };

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
    model->descriptor_heap = Game::Get()->GetEngine()->CreateDescriptorHeap(materials.size() * 3);

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

    return std::move(model);
}
