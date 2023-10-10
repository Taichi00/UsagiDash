#include "Scene.h"
#include "Engine.h"
#include "App.h"
#include "d3dx12.h"
#include "SharedStruct.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "ConstantBuffer.h"
#include "RootSignature.h"
#include "PipelineState.h"
#include "AssimpLoader.h"
#include "DescriptorHeap.h"
#include "Texture2D.h"

Scene* g_Scene;

using namespace DirectX;

ConstantBuffer* constantBuffer[Engine::FRAME_BUFFER_COUNT];
RootSignature* rootSignature;
PipelineState* pipelineState;
PipelineState* pipelineState2;

//const wchar_t* modelFile = L"Assets/Alicia/FBX/Alicia_solid_Unity.FBX";
const wchar_t* modelFile = L"Assets/�~�ێ������~�NNT_ver1.1.2/FBX/mikuNT.glb";
//const wchar_t* modelFile = L"Assets/�~�ێ������~�NNT_ver1.1.2/FBX/HatsuneMikuNT2.fbx";
std::vector<Mesh> meshes;
std::vector<Material> materials;

DescriptorHeap* descriptorHeap;
std::vector<DescriptorHandle*> materialHandles;	// �e�N�X�`���p�̃n���h���ꗗ

bool Scene::Init()
{
	ImportSettings importSetting =
	{
		modelFile,
		meshes,
		materials,
		false,
		true
	};

	AssimpLoader loader;
	if (!loader.Load(importSetting))
	{
		return false;
	}

	auto eyePos		= XMVectorSet(0.0f, 130.0f, 150.0f, 0.0f);	// ���_�̈ʒu
	auto targetPos	= XMVectorSet(0.0f, 90.0f, 0.0f, 0.0f);	// ���_����������W
	auto upward		= XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);		// �������\���x�N�g��
	auto fov		= XMConvertToRadians(60.0f);					// ����p
	auto aspect		= static_cast<float>(WINDOW_WIDTH) / static_cast<float>(WINDOW_HEIGHT); // �A�X�y�N�g��

	for (size_t i = 0; i < Engine::FRAME_BUFFER_COUNT; i++)
	{
		constantBuffer[i] = new ConstantBuffer(sizeof(Transform));
		if (!constantBuffer[i]->IsValid())
		{
			printf("�ϊ��s��p�萔�o�b�t�@�̐����Ɏ��s\n");
			return false;
		}

		// �ϊ��s��̓o�^
		auto ptr = constantBuffer[i]->GetPtr<Transform>();
		ptr->World = XMMatrixIdentity();
		ptr->View  = XMMatrixLookAtRH(eyePos, targetPos, upward);
		ptr->Proj  = XMMatrixPerspectiveFovRH(fov, aspect, 0.3f, 1000.0f);
	}

	// �}�e���A���̓ǂݍ���
	materialHandles.clear();
	descriptorHeap = new DescriptorHeap();
	for (size_t i = 0; i < meshes.size(); i++)
	{
		auto mainTex = materials[meshes[i].MaterialIndex].Texture;
		auto handle = descriptorHeap->Register(mainTex);
		materialHandles.push_back(handle);
	}

	rootSignature = new RootSignature();
	if (!rootSignature->IsValid())
	{
		printf("���[�g�V�O�l�`���̐����Ɏ��s\n");
		return false;
	}

	pipelineState = new PipelineState();
	pipelineState->SetInputLayout(Vertex::InputLayout);
	pipelineState->SetRootSignature(rootSignature->Get());
	pipelineState->SetVS(L"../x64/Debug/SimpleVS.cso");
	pipelineState->SetPS(L"../x64/Debug/SimplePS.cso");
	pipelineState->SetCullMode(D3D12_CULL_MODE_NONE);
	pipelineState->Create();
	if (!pipelineState->IsValid())
	{
		printf("�p�C�v���C���X�e�[�g�̐����Ɏ��s\n");
		return false;
	}

	pipelineState2 = new PipelineState();
	pipelineState2->SetInputLayout(Vertex::InputLayout);
	pipelineState2->SetRootSignature(rootSignature->Get());
	pipelineState2->SetVS(L"../x64/Debug/OutlineVS.cso");
	pipelineState2->SetPS(L"../x64/Debug/OutlinePS.cso");
	pipelineState2->SetCullMode(D3D12_CULL_MODE_BACK);
	pipelineState2->Create();
	if (!pipelineState2->IsValid())
	{
		printf("�p�C�v���C���X�e�[�g�̐����Ɏ��s\n");
		return false;
	}

	printf("�V�[���̏������ɐ���\n");
	return true;
}

void Scene::Update()
{
	auto currentIndex = g_Engine->CurrentBackBufferIndex();
	auto currentTransform = constantBuffer[currentIndex]->GetPtr<Transform>();

	auto world = XMMatrixIdentity();
	world *= XMMatrixScaling(100, 100, 100);

	currentTransform->World = world;
}

void Scene::Draw()
{
	auto currentIndex = g_Engine->CurrentBackBufferIndex();
	auto commandList = g_Engine->CommandList();
	auto materialHeap = descriptorHeap->GetHeap();

	for (size_t i = 0; i < meshes.size(); i++)
	{
		auto vbView = meshes[i].pVertexBuffer->View();
		auto ibView = meshes[i].pIndexBuffer->View();

		commandList->SetGraphicsRootSignature(rootSignature->Get());	// ���[�g�V�O�l�`�����Z�b�g
		commandList->SetPipelineState(pipelineState->Get());			// �p�C�v���C���X�e�[�g���Z�b�g
		commandList->SetGraphicsRootConstantBufferView(0, constantBuffer[currentIndex]->GetAddress());	// �萔�o�b�t�@���Z�b�g

		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		commandList->IASetVertexBuffers(0, 1, &vbView);					// ���_�o�b�t�@���X���b�g0�Ԃ��g����1�����ݒ肷��
		commandList->IASetIndexBuffer(&ibView);							// �C���f�b�N�X�o�b�t�@���Z�b�g���� 

		commandList->SetDescriptorHeaps(1, &materialHeap);				// �f�B�X�N���v�^�q�[�v���Z�b�g
		commandList->SetGraphicsRootDescriptorTable(1, materialHandles[i]->HandleGPU);	// �f�B�X�N���v�^�e�[�u�����Z�b�g

		commandList->DrawIndexedInstanced(meshes[i].Indices.size(), 1, 0, 0, 0);
	}

	for (size_t i = 0; i < meshes.size(); i++)
	{
		auto vbView = meshes[i].pVertexBuffer->View();
		auto ibView = meshes[i].pIndexBuffer->View();

		commandList->SetGraphicsRootSignature(rootSignature->Get());	// ���[�g�V�O�l�`�����Z�b�g
		commandList->SetPipelineState(pipelineState2->Get());			// �p�C�v���C���X�e�[�g���Z�b�g
		commandList->SetGraphicsRootConstantBufferView(0, constantBuffer[currentIndex]->GetAddress());	// �萔�o�b�t�@���Z�b�g

		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		commandList->IASetVertexBuffers(0, 1, &vbView);					// ���_�o�b�t�@���X���b�g0�Ԃ��g����1�����ݒ肷��
		commandList->IASetIndexBuffer(&ibView);							// �C���f�b�N�X�o�b�t�@���Z�b�g���� 

		commandList->SetDescriptorHeaps(1, &materialHeap);				// �f�B�X�N���v�^�q�[�v���Z�b�g
		commandList->SetGraphicsRootDescriptorTable(1, materialHandles[i]->HandleGPU);	// �f�B�X�N���v�^�e�[�u�����Z�b�g

		commandList->DrawIndexedInstanced(meshes[i].Indices.size(), 1, 0, 0, 0);
	}
}

