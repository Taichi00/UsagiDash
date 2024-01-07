#pragma once
#include "Vec.h"
#include <DirectXMath.h>
#include <d3d12.h>
#include <vector>
#include <utility>

using namespace DirectX;

struct Vertex
{
	XMFLOAT3 Position;	// �ʒu���W
	XMFLOAT3 Normal;	// �@��
	XMFLOAT2 UV;		// uv���W
	XMFLOAT3 Tangent;	// �ڋ��
	XMFLOAT4 Color;		// ���_�F
	XMUINT4  BoneIndices;
	XMFLOAT4 BoneWeights;
	UINT BoneNum = 0;
	XMFLOAT3 SmoothNormal;	// �A�E�g���C���p�̃X���[�Y�Ȗ@��

	static const D3D12_INPUT_LAYOUT_DESC InputLayout;
private:
	static const int InputElementCount = 9;
	static const D3D12_INPUT_ELEMENT_DESC InputElements[InputElementCount];
};

struct alignas(256) TransformParameter
{
	XMMATRIX World;		// ���[���h�s��
	XMMATRIX View;		// �r���[�s��
	XMMATRIX Proj;		// ���e�s��
	float DitherLevel;	// �f�B�U�����O�̃��x��
};

struct alignas(256) SceneParameter
{
	//XMVECTOR ScreenSize;
	XMMATRIX LightView;
	XMMATRIX LightProj;
	XMVECTOR LightColor;
	XMVECTOR LightDirection;
	XMVECTOR CameraPosition;
	XMMATRIX LightWorld;
};

struct alignas(256) BoneParameter
{
	XMFLOAT4X4 bone[512];
	XMFLOAT4X4 boneNormal[512];
};

struct alignas(256) MaterialParameter
{
	XMVECTOR BaseColor;
	float Shininess;
	float OutlineWidth;
};

struct CollisionVertex
{
	Vec3 Position;	// �ʒu���W
	Vec3 Normal;	// �@��
};

struct CollisionFace
{
	uint32_t Indices[3];
	Vec3 Normal;
	std::vector<std::pair<uint32_t, uint32_t>> Edges;
};

struct CollisionMesh
{
	std::vector<CollisionVertex> Vertices;
	std::vector<CollisionFace> Faces;
	//std::vector<uint32_t> Indices;
	//std::vector<bool> edgeIgnoreFlags;	// �G�b�W�𖳎����邩�ǂ���
};

struct CollisionModel
{
	std::vector<CollisionMesh> Meshes;
};