#pragma once

#include "game/bone_list.h"
#include "engine/descriptor_heap.h"
#include "game/resource.h"
#include "engine/shared_struct.h"
#include <DirectXMath.h>
#include <memory>
#include <string>
#include <vector>

class VertexBuffer;
class IndexBuffer;
class Texture2D;
class Animation;

struct Mesh
{
	std::vector<Vertex> vertices;	// 頂点データの配列
	std::vector<uint32_t> indices;	// インデックスの配列
	std::shared_ptr<VertexBuffer> vertex_buffer;		// 頂点バッファへのポインタ
	std::shared_ptr<IndexBuffer> index_buffer;		// インデックスバッファへのポインタ
	int material_index;				// マテリアルのインデックス
};

struct Material
{
	std::shared_ptr<Texture2D> albedo_texture;		// アルベドテクスチャ
	std::shared_ptr<Texture2D> pbr_texture;			// メタリック・ラフネステクスチャ
	std::shared_ptr<Texture2D> normal_texture;		// ノーマルテクスチャ
	DescriptorHandle albedo_handle;	// アルベドテクスチャへのディスクリプタハンドル
	DescriptorHandle pbr_handle;		// メタリック・ラフネステクスチャへのディスクリプタハンドル
	DescriptorHandle normal_handle;	// ノーマルテクスチャへのディスクリプタハンドル
	int alpha_mode;					// 半透明マテリアルかどうか
	XMVECTOR base_color;				// ベースカラー
};

class Model : public Resource
{
public:
	Model();
	~Model();

	static std::unique_ptr<Model> Load(const std::string& key);
	bool Release() override;

public:
	std::vector<Mesh> meshes;
	std::vector<Material> materials;
	std::vector<std::shared_ptr<Animation>> animations;
	BoneList bones;
	std::shared_ptr<DescriptorHeap> descriptor_heap;
};