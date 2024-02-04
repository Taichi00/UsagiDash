#pragma once
#include "math/vec.h"
#include <DirectXMath.h>
#include <d3d12.h>
#include <vector>
#include <utility>

using namespace DirectX;

struct Vertex
{
	XMFLOAT3 position;	// 位置座標
	XMFLOAT3 normal;	// 法線
	XMFLOAT2 uv;		// uv座標
	XMFLOAT3 tangent;	// 接空間
	XMFLOAT4 color;		// 頂点色
	XMUINT4  bone_indices;
	XMFLOAT4 bone_weights;
	UINT bone_num = 0;
	XMFLOAT3 smooth_normal;	// アウトライン用のスムーズな法線

	static const D3D12_INPUT_LAYOUT_DESC InputLayout;
private:
	static const int InputElementCount = 9;
	static const D3D12_INPUT_ELEMENT_DESC InputElements[InputElementCount];
};

struct alignas(256) TransformParameter
{
	XMMATRIX world;		// ワールド行列
	XMMATRIX view;		// ビュー行列
	XMMATRIX proj;		// 投影行列
	float dither_level;	// ディザリングのレベル
};

struct alignas(256) SceneParameter
{
	//XMVECTOR ScreenSize;
	XMMATRIX light_view;
	XMMATRIX light_proj;
	XMVECTOR light_color;
	XMVECTOR light_direction;
	XMVECTOR camera_position;
	XMMATRIX light_world;
};

struct alignas(256) BoneParameter
{
	XMFLOAT4X4 bone[512];
	XMFLOAT4X4 bone_normal[512];
};

struct alignas(256) MaterialParameter
{
	XMVECTOR base_color;
	float shininess;
	float outline_width;
};
