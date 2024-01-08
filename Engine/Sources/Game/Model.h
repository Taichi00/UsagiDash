#pragma once

#include "BoneList.h"
#include "DescriptorHeap.h"
#include "Resource.h"
#include "SharedStruct.h"
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
	std::shared_ptr<VertexBuffer> vertexBuffer;		// 頂点バッファへのポインタ
	std::shared_ptr<IndexBuffer> indexBuffer;		// インデックスバッファへのポインタ
	int materialIndex;				// マテリアルのインデックス
};

struct Material
{
	std::shared_ptr<Texture2D> albedoTexture;		// アルベドテクスチャ
	std::shared_ptr<Texture2D> pbrTexture;			// メタリック・ラフネステクスチャ
	std::shared_ptr<Texture2D> normalTexture;		// ノーマルテクスチャ
	DescriptorHandle albedoHandle;	// アルベドテクスチャへのディスクリプタハンドル
	DescriptorHandle pbrHandle;		// メタリック・ラフネステクスチャへのディスクリプタハンドル
	DescriptorHandle normalHandle;	// ノーマルテクスチャへのディスクリプタハンドル
	int alphaMode;					// 半透明マテリアルかどうか
	XMVECTOR baseColor;				// ベースカラー
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
	std::unique_ptr<DescriptorHeap> descriptorHeap;
};