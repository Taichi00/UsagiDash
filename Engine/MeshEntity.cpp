#include "MeshEntity.h"
#include "AssimpLoader.h"
#include "App.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Window.h"
#include "Texture2D.h"
#include "Vec.h"
#include "ShadowMap.h"
#include "Bone.h"
#include "BoneList.h"
#include "Animation.h"
#include "Animator.h"
#include <DirectXMath.h>

using namespace DirectX;


MeshEntity::MeshEntity(std::vector<Mesh> meshes, std::vector<Material> materials,
	BoneList bones, std::vector<Animation*> animations) : Entity()
{
	m_meshes = meshes;
	m_materials = materials;
	m_bones = bones;

	/*m_pAnimator = new Animator(bones);
	m_pAnimator->RegisterAnimations(animations);*/
}

MeshEntity::~MeshEntity()
{
	delete[] m_pTransformCB;
	delete[] m_pLightCB;
	delete[] m_pBoneCB;
	delete m_pRootSignature;
	delete m_pOpaquePSO;
	delete m_pAlphaPSO;
	delete m_pOutlinePSO;
	delete m_pShadowPSO;
	delete m_pDescriptorHeap;
	delete m_pShadowHandle;

	delete m_pAnimator;
}

MeshEntity* MeshEntity::LoadModel(const wchar_t* path)
{
	/*std::vector<Mesh> meshes;
	std::vector<Material> materials;
	BoneList bones = BoneList();*/
	Model model;
	std::vector<Animation*> animations;

	AssimpLoader loader;
	if (!loader.Load(path, model, animations))
	{
		return nullptr;
	}

	return new MeshEntity(model.Meshes, model.Materials, model.Bones, animations);
}

bool MeshEntity::Init()
{
	for (size_t i = 0; i < Engine::FRAME_BUFFER_COUNT; i++)
	{
		m_pTransformCB[i] = new ConstantBuffer(sizeof(Transform));
		if (!m_pTransformCB[i]->IsValid())
		{
			printf("変換行列用定数バッファの生成に失敗\n");
			return false;
		}

		// 変換行列の登録
		auto ptr = m_pTransformCB[i]->GetPtr<Transform>();
		ptr->World = XMMatrixIdentity();
		ptr->View = XMMatrixIdentity();
		ptr->Proj = XMMatrixIdentity();
	}

	const auto eye = XMVectorSet(10.0f, 150.0f, 50.0f, 0.0f);
	const auto target = XMVectorSet(0.0f, 90.0f, 0.0f, 0.0f);
	const auto up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	auto dir = Vec3(0.0f - 10.0f, 90.0f - 150.0f, 0.0f - 50.0f).normalized();
	
	for (size_t i = 0; i < Engine::FRAME_BUFFER_COUNT; i++)
	{
		m_pLightCB[i] = new ConstantBuffer(sizeof(Light));
		if (!m_pLightCB[i]->IsValid())
		{
			printf("ライト用定数バッファの生成に失敗\n");
			return false;
		}

		auto ptr = m_pLightCB[i]->GetPtr<Light>();
		ptr->Direction = XMFLOAT3(dir.x, dir.y, dir.z);
		ptr->View = XMMatrixLookAtRH(eye, target, up);
		ptr->Proj = XMMatrixOrthographicRH(500, 500, 0.1f, 1000.0f);
	}

	for (size_t i = 0; i < Engine::FRAME_BUFFER_COUNT; i++)
	{
		m_pBoneCB[i] = new ConstantBuffer(sizeof(BoneParameter));
		if (!m_pBoneCB[i]->IsValid())
		{
			printf("ボーン用定数バッファの生成に失敗\n");
			return false;
		}
	}

	// マテリアルの読み込み
	D3D12_DESCRIPTOR_HEAP_DESC desc{};
	desc.NodeMask = 1;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	desc.NumDescriptors = m_materials.size() + 1;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	m_pDescriptorHeap = new DescriptorHeap(desc);
	for (size_t i = 0; i < m_materials.size(); i++)
	{
		auto texture = m_materials[i].Texture;
		auto pHandle = m_pDescriptorHeap->Alloc();
		auto resource = texture->Resource();
		auto desc = texture->ViewDesc();
		g_Engine->Device()->CreateShaderResourceView(resource, &desc, pHandle->HandleCPU());	// シェーダーリソースビュー作成

		m_materials[i].pHandle = pHandle;
		m_materials[i].pPipelineState = m_pOpaquePSO;
	}

	// シャドウマップの登録
	auto pShadowMap = m_pGame->GetShadowMap();

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

	m_pShadowHandle = m_pDescriptorHeap->Alloc();
	g_Engine->Device()->CreateShaderResourceView(pShadowMap->Resource(), &srvDesc, m_pShadowHandle->HandleCPU());


	m_pRootSignature = new RootSignature();
	if (!m_pRootSignature->IsValid())
	{
		printf("ルートシグネチャの生成に失敗\n");
		return false;
	}

	if (!PreparePSO())
	{
		printf("パイプラインステートの生成に失敗\n");
		return false;
	}

	printf("エンティティの初期化に成功\n");
	return true;
}

void MeshEntity::Update()
{
	UpdateAnimator();
	UpdateBone();
	UpdateCB();
}

void MeshEntity::Draw()
{
	auto currentIndex = g_Engine->CurrentBackBufferIndex();
	auto commandList = g_Engine->CommandList();
	auto materialHeap = m_pDescriptorHeap->GetHeap();

	commandList->SetGraphicsRootSignature(m_pRootSignature->Get());	// ルートシグネチャをセット
	commandList->SetGraphicsRootConstantBufferView(0, m_pTransformCB[currentIndex]->GetAddress());	// 定数バッファをセット
	commandList->SetGraphicsRootConstantBufferView(1, m_pLightCB[currentIndex]->GetAddress());
	commandList->SetGraphicsRootConstantBufferView(2, m_pBoneCB[currentIndex]->GetAddress());
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	
	ID3D12DescriptorHeap* heaps[] = {
		m_pDescriptorHeap->GetHeap(),
	};
	commandList->SetDescriptorHeaps(1, heaps);				// ディスクリプタヒープをセット
	commandList->SetGraphicsRootDescriptorTable(4, m_pShadowHandle->HandleGPU());	// ディスクリプタテーブルをセット

	// メッシュの描画
	for (Mesh mesh : m_meshes)
	{
		auto vbView = mesh.pVertexBuffer->View();
		auto ibView = mesh.pIndexBuffer->View();
		auto mat = m_materials[mesh.MaterialIndex];
		auto alphaMode = mat.AlphaMode;

		ID3D12PipelineState* pso;
		switch (alphaMode)
		{
		case 1:
			continue;
		default:
			pso = m_pOpaquePSO->Get();
			break;
		}

		commandList->SetPipelineState(pso);								// パイプラインステートをセット
		commandList->IASetVertexBuffers(0, 1, &vbView);					// 頂点バッファをスロット0番を使って1個だけ設定する
		commandList->IASetIndexBuffer(&ibView);							// インデックスバッファをセット
		commandList->SetGraphicsRootDescriptorTable(3, mat.pHandle->HandleGPU());	// ディスクリプタテーブルをセット

		commandList->DrawIndexedInstanced(mesh.Indices.size(), 1, 0, 0, 0);
	}

	// アウトラインの描画
	for (Mesh mesh : m_meshes)
	{
		auto vbView = mesh.pVertexBuffer->View();
		auto ibView = mesh.pIndexBuffer->View();
		auto mat = m_materials[mesh.MaterialIndex];
		auto alphaMode = mat.AlphaMode;

		if (alphaMode == 1)
		{
			continue;
		}

		commandList->SetPipelineState(m_pOutlinePSO->Get());			// パイプラインステートをセット
		commandList->IASetVertexBuffers(0, 1, &vbView);					// 頂点バッファをスロット0番を使って1個だけ設定する
		commandList->IASetIndexBuffer(&ibView);							// インデックスバッファをセットする 
		commandList->SetGraphicsRootDescriptorTable(3, mat.pHandle->HandleGPU());	// ディスクリプタテーブルをセット

		commandList->DrawIndexedInstanced(mesh.Indices.size(), 1, 0, 0, 0);
	}
}

void MeshEntity::DrawAlpha()
{
	auto currentIndex = g_Engine->CurrentBackBufferIndex();
	auto commandList = g_Engine->CommandList();
	auto materialHeap = m_pDescriptorHeap->GetHeap();

	commandList->SetGraphicsRootSignature(m_pRootSignature->Get());	// ルートシグネチャをセット
	commandList->SetGraphicsRootConstantBufferView(0, m_pTransformCB[currentIndex]->GetAddress());	// 定数バッファをセット
	commandList->SetGraphicsRootConstantBufferView(1, m_pLightCB[currentIndex]->GetAddress());
	commandList->SetGraphicsRootConstantBufferView(2, m_pBoneCB[currentIndex]->GetAddress());
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	ID3D12DescriptorHeap* heaps[] = {
		m_pDescriptorHeap->GetHeap(),
	};
	commandList->SetDescriptorHeaps(1, heaps);				// ディスクリプタヒープをセット
	commandList->SetGraphicsRootDescriptorTable(4, m_pShadowHandle->HandleGPU());	// ディスクリプタテーブルをセット

	// メッシュの描画
	for (Mesh mesh : m_meshes)
	{
		auto vbView = mesh.pVertexBuffer->View();
		auto ibView = mesh.pIndexBuffer->View();
		auto mat = m_materials[mesh.MaterialIndex];
		auto alphaMode = mat.AlphaMode;

		ID3D12PipelineState* pso;
		switch (alphaMode)
		{
		case 1:
			pso = m_pAlphaPSO->Get();
			break;
		default:
			continue;
		}

		commandList->SetPipelineState(pso);								// パイプラインステートをセット
		commandList->IASetVertexBuffers(0, 1, &vbView);					// 頂点バッファをスロット0番を使って1個だけ設定する
		commandList->IASetIndexBuffer(&ibView);							// インデックスバッファをセット
		commandList->SetGraphicsRootDescriptorTable(3, mat.pHandle->HandleGPU());	// ディスクリプタテーブルをセット

		commandList->DrawIndexedInstanced(mesh.Indices.size(), 1, 0, 0, 0);
	}
}

void MeshEntity::DrawShadow()
{
	auto currentIndex = g_Engine->CurrentBackBufferIndex();
	auto commandList = g_Engine->CommandList();

	commandList->SetGraphicsRootSignature(m_pRootSignature->Get());	// ルートシグネチャをセット
	commandList->SetGraphicsRootConstantBufferView(0, m_pTransformCB[currentIndex]->GetAddress());	// 定数バッファをセット
	commandList->SetGraphicsRootConstantBufferView(1, m_pLightCB[currentIndex]->GetAddress());
	commandList->SetGraphicsRootConstantBufferView(2, m_pBoneCB[currentIndex]->GetAddress());
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	ID3D12DescriptorHeap* heaps[] = {
		m_pDescriptorHeap->GetHeap(),
	};
	commandList->SetDescriptorHeaps(1, heaps);				// ディスクリプタヒープをセット
	//commandList->SetGraphicsRootDescriptorTable(3, m_pShadowHandle->HandleGPU());	// ディスクリプタテーブルをセット

	// メッシュの描画
	for (Mesh mesh : m_meshes)
	{
		auto vbView = mesh.pVertexBuffer->View();
		auto ibView = mesh.pIndexBuffer->View();
		auto mat = m_materials[mesh.MaterialIndex];

		ID3D12PipelineState* pso = m_pShadowPSO->Get();
		
		commandList->SetPipelineState(pso);								// パイプラインステートをセット
		commandList->IASetVertexBuffers(0, 1, &vbView);					// 頂点バッファをスロット0番を使って1個だけ設定する
		commandList->IASetIndexBuffer(&ibView);							// インデックスバッファをセットする 
		commandList->SetGraphicsRootDescriptorTable(3, mat.pHandle->HandleGPU());	// ディスクリプタテーブルをセット

		commandList->DrawIndexedInstanced(mesh.Indices.size(), 1, 0, 0, 0);
	}
}

bool MeshEntity::PreparePSO()
{
	m_pOpaquePSO = new PipelineState();
	m_pOpaquePSO->SetInputLayout(Vertex::InputLayout);
	m_pOpaquePSO->SetRootSignature(m_pRootSignature->Get());
	m_pOpaquePSO->SetVS(L"../x64/Debug/SimpleVS.cso");
	m_pOpaquePSO->SetPS(L"../x64/Debug/SimplePS.cso");
	m_pOpaquePSO->SetCullMode(D3D12_CULL_MODE_NONE);
	m_pOpaquePSO->Create();
	if (!m_pOpaquePSO->IsValid())
	{
		return false;
	}

	m_pAlphaPSO = new PipelineState();
	m_pAlphaPSO->SetInputLayout(Vertex::InputLayout);
	m_pAlphaPSO->SetRootSignature(m_pRootSignature->Get());
	m_pAlphaPSO->SetVS(L"../x64/Debug/SimpleVS.cso");
	m_pAlphaPSO->SetPS(L"../x64/Debug/AlphaPS.cso");
	m_pAlphaPSO->SetCullMode(D3D12_CULL_MODE_NONE);
	m_pAlphaPSO->SetAlpha();
	m_pAlphaPSO->Create();
	if (!m_pAlphaPSO->IsValid())
	{
		return false;
	}

	m_pOutlinePSO = new PipelineState();
	m_pOutlinePSO->SetInputLayout(Vertex::InputLayout);
	m_pOutlinePSO->SetRootSignature(m_pRootSignature->Get());
	m_pOutlinePSO->SetVS(L"../x64/Debug/OutlineVS.cso");
	m_pOutlinePSO->SetPS(L"../x64/Debug/OutlinePS.cso");
	m_pOutlinePSO->SetCullMode(D3D12_CULL_MODE_BACK);
	m_pOutlinePSO->Create();
	if (!m_pOutlinePSO->IsValid())
	{
		return false;
	}

	m_pShadowPSO = new PipelineState();
	m_pShadowPSO->SetInputLayout(Vertex::InputLayout);
	m_pShadowPSO->SetRootSignature(m_pRootSignature->Get());
	m_pShadowPSO->SetVS(L"../x64/Debug/ShadowVS.cso");
	m_pShadowPSO->SetPS(L"../x64/Debug/ShadowPS.cso");
	m_pShadowPSO->SetCullMode(D3D12_CULL_MODE_NONE);
	m_pShadowPSO->Create();
	if (!m_pShadowPSO->IsValid())
	{
		return false;
	}
}

Animator* MeshEntity::GetAnimator()
{
	return m_pAnimator;
}

void MeshEntity::UpdateAnimator()
{
	m_pAnimator->Update();
}

void MeshEntity::UpdateBone()
{
	// 行列の更新
	for (auto b : m_bones)
	{
		if (b->GetParent())
		{
			continue;
		}

		b->UpdateMatrices();
	}
}

void MeshEntity::UpdateCB()
{
	auto currentIndex = g_Engine->CurrentBackBufferIndex();

	// Transform
	auto currentTransform = m_pTransformCB[currentIndex]->GetPtr<Transform>();

	// world行列
	auto world = XMMatrixIdentity();
	world *= XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z);
	world *= XMMatrixRotationZ(m_rotation.z);
	world *= XMMatrixRotationY(m_rotation.y);
	world *= XMMatrixRotationX(m_rotation.x);
	world *= XMMatrixTranslation(m_position.x, m_position.y, m_position.z);

	// view行列
	auto view = m_pGame->GetViewMatrix();

	// proj行列
	auto proj = m_pGame->GetProjMatrix();

	currentTransform->World = world;
	currentTransform->View = view;
	currentTransform->Proj = proj;

	// Bone
	auto currentBone = m_pBoneCB[currentIndex]->GetPtr<BoneParameter>();
	
	for (size_t i = 0; i < m_bones.Size(); i++)
	{
		auto bone = m_bones[i];
		auto mtx = bone->GetWorldMatrix() * bone->GetInvBindMatrix();
		XMStoreFloat4x4(&(currentBone->bone[i]), XMMatrixTranspose(mtx));
	}
}
