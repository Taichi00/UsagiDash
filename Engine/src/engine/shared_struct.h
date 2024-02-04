#pragma once
#include "math/vec.h"
#include <DirectXMath.h>
#include <d3d12.h>
#include <vector>
#include <utility>

using namespace DirectX;

struct Vertex
{
	XMFLOAT3 position;	// �ʒu���W
	XMFLOAT3 normal;	// �@��
	XMFLOAT2 uv;		// uv���W
	XMFLOAT3 tangent;	// �ڋ��
	XMFLOAT4 color;		// ���_�F
	XMUINT4  bone_indices;
	XMFLOAT4 bone_weights;
	UINT bone_num = 0;
	XMFLOAT3 smooth_normal;	// �A�E�g���C���p�̃X���[�Y�Ȗ@��

	static const D3D12_INPUT_LAYOUT_DESC InputLayout;
private:
	static const int InputElementCount = 9;
	static const D3D12_INPUT_ELEMENT_DESC InputElements[InputElementCount];
};

struct alignas(256) TransformParameter
{
	XMMATRIX world;		// ���[���h�s��
	XMMATRIX view;		// �r���[�s��
	XMMATRIX proj;		// ���e�s��
	float dither_level;	// �f�B�U�����O�̃��x��
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
