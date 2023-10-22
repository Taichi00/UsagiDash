#pragma once
#include "d3dx12.h"
#include <DirectXMath.h>
#include "ComPtr.h"
#include "Engine.h"
#include "BoneList.h"
#include "Vec.h"
#include <string>

class Texture2D;
class VertexBuffer;
class IndexBuffer;
class ConstantBuffer;
class PipelineState;
class DescriptorHandle;
class DescriptorHeap;
class RootSignature;
class Bone;

using namespace DirectX;

struct Vertex
{
	XMFLOAT3 Position;	// 位置座標
	XMFLOAT3 Normal;	// 法線
	XMFLOAT2 UV;		// uv座標
	XMFLOAT3 Tangent;	// 接空間
	XMFLOAT4 Color;		// 頂点色
	XMUINT4  BoneIndices;
	XMFLOAT4 BoneWeights;
	UINT BoneNum = 0;
	XMFLOAT3 SmoothNormal;	// アウトライン用のスムーズな法線

	static const D3D12_INPUT_LAYOUT_DESC InputLayout;
private:
	static const int InputElementCount = 9;
	static const D3D12_INPUT_ELEMENT_DESC InputElements[InputElementCount];
};

struct alignas(256) TransformParameter
{
	XMMATRIX World;		// ワールド行列
	XMMATRIX View;		// ビュー行列
	XMMATRIX Proj;		// 投影行列
};

struct alignas(256) SceneParameter
{
	XMMATRIX LightView;
	XMMATRIX LightProj;
	XMVECTOR LightColor;
	XMVECTOR LightDirection;
	XMVECTOR CameraPosition;
};

struct alignas(256) BoneParameter
{
	XMFLOAT4X4 bone[512];
};

struct alignas(256) MaterialParameter
{
	XMVECTOR BaseColor;
	float Shininess;
	float OutlineWidth;
};

struct Mesh
{
	std::vector<Vertex> Vertices;	// 頂点データの配列
	std::vector<uint32_t> Indices;	// インデックスの配列
	VertexBuffer* pVertexBuffer;	// 頂点バッファへのポインタ
	IndexBuffer* pIndexBuffer;		// インデックスバッファへのポインタ
	int MaterialIndex;
};

struct Material
{
	Texture2D* Texture;
	DescriptorHandle* pHandle;
	PipelineState* pPipelineState;
	int AlphaMode;
	XMVECTOR BaseColor;
	float Shininess;
};

struct Model
{
	std::vector<Mesh> Meshes;
	std::vector<Material> Materials;
	BoneList Bones;
};