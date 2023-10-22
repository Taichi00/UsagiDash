#include "MeshRenderer.h"
#include "ConstantBuffer.h"
#include "DescriptorHeap.h"
#include "Texture2D.h"
#include "Entity.h"
#include "Scene.h"
#include "ShadowMap.h"
#include "RootSignature.h"
#include "PipelineState.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Camera.h"
#include "Bone.h"


MeshRenderer::MeshRenderer(MeshRendererProperty prop)
{
	SetProperties(prop);
}

MeshRenderer::~MeshRenderer()
{
	delete[] m_pTransformCB;
	delete[] m_pSceneCB;
	delete[] m_pBoneCB;
	delete m_pRootSignature;
	delete m_pOpaquePSO;
	delete m_pAlphaPSO;
	delete m_pOutlinePSO;
	delete m_pShadowPSO;
	delete m_pDescriptorHeap;
	delete m_pShadowHandle;
}

void MeshRenderer::SetProperties(MeshRendererProperty prop)
{
	m_model = prop.Model;
	m_model.Bones = BoneList::Copy(m_model.Bones);
}

bool MeshRenderer::Init()
{
	for (size_t i = 0; i < Engine::FRAME_BUFFER_COUNT; i++)
	{
		m_pTransformCB[i] = new ConstantBuffer(sizeof(TransformParameter));
		if (!m_pTransformCB[i]->IsValid())
		{
			printf("変換行列用定数バッファの生成に失敗\n");
			return false;
		}

		//auto ptr = m_pTransformCB[i]->GetPtr<Transform>();
		//ptr->World = XMMatrixIdentity();
		//ptr->View = XMMatrixIdentity();
		//ptr->Proj = XMMatrixIdentity();
	}

	const auto eye = XMVectorSet(0.5f, 7.5f, 2.5f, 0.0f);
	const auto target = XMVectorSet(0.0f, 4.0f, 0.0f, 0.0f);
	const auto up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	auto dir = Vec3(0.0f - 0.5f, 4.0f - 7.5f, 0.0f - 2.5f).normalized();

	for (size_t i = 0; i < Engine::FRAME_BUFFER_COUNT; i++)
	{
		m_pSceneCB[i] = new ConstantBuffer(sizeof(SceneParameter));
		if (!m_pSceneCB[i]->IsValid())
		{
			printf("ライト用定数バッファの生成に失敗\n");
			return false;
		}

		auto ptr = m_pSceneCB[i]->GetPtr<SceneParameter>();
		ptr->LightDirection = { dir.x, dir.y, dir.z };
		ptr->LightView = XMMatrixLookAtRH(eye, target, up);
		ptr->LightProj = XMMatrixOrthographicRH(25, 25, 0.1f, 100.0f);
		ptr->CameraPosition = { 0, 0, 1 };
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

	for (size_t i = 0; i < m_model.Materials.size(); i++)
	{
		ConstantBuffer* cb = new ConstantBuffer(sizeof(MaterialParameter));
		if (!cb->IsValid())
		{
			printf("マテリアル用定数バッファの生成に失敗\n");
			return false;
		}

		auto mat = &m_model.Materials[i];
		auto ptr = cb->GetPtr<MaterialParameter>();
		ptr->BaseColor = mat->BaseColor;
		ptr->Shininess = mat->Shininess;
		ptr->OutlineWidth = 0.004;

		m_pMaterialCBs.push_back(cb);
	}

	// マテリアルの読み込み
	D3D12_DESCRIPTOR_HEAP_DESC desc{};
	desc.NodeMask = 1;
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	desc.NumDescriptors = m_model.Materials.size() + 1;
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	m_pDescriptorHeap = new DescriptorHeap(desc);
	for (size_t i = 0; i < m_model.Materials.size(); i++)
	{
		auto texture = m_model.Materials[i].Texture;
		auto pHandle = m_pDescriptorHeap->Alloc();
		auto resource = texture->Resource();
		auto desc = texture->ViewDesc();
		g_Engine->Device()->CreateShaderResourceView(resource, &desc, pHandle->HandleCPU());	// シェーダーリソースビュー作成

		m_model.Materials[i].pHandle = pHandle;
		m_model.Materials[i].pPipelineState = m_pOpaquePSO;
	}

	// シャドウマップの登録
	auto pShadowMap = m_pEntity->GetScene()->GetShadowMap();

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

	printf("MeshRendererの初期化に成功\n");
	return true;
}

void MeshRenderer::Update()
{
	UpdateBone();
	UpdateCB();
}

void MeshRenderer::Draw()
{
	auto currentIndex = g_Engine->CurrentBackBufferIndex();
	auto commandList = g_Engine->CommandList();
	auto materialHeap = m_pDescriptorHeap->GetHeap();

	commandList->SetGraphicsRootSignature(m_pRootSignature->Get());	// ルートシグネチャをセット
	commandList->SetGraphicsRootConstantBufferView(0, m_pTransformCB[currentIndex]->GetAddress());	// 定数バッファをセット
	commandList->SetGraphicsRootConstantBufferView(1, m_pSceneCB[currentIndex]->GetAddress());
	commandList->SetGraphicsRootConstantBufferView(2, m_pBoneCB[currentIndex]->GetAddress());
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	
	ID3D12DescriptorHeap* heaps[] = {
		m_pDescriptorHeap->GetHeap(),
	};
	commandList->SetDescriptorHeaps(1, heaps);				// ディスクリプタヒープをセット
	commandList->SetGraphicsRootDescriptorTable(5, m_pShadowHandle->HandleGPU());	// ディスクリプタテーブルをセット

	// メッシュの描画
	for (Mesh mesh : m_model.Meshes)
	{
		auto vbView = mesh.pVertexBuffer->View();
		auto ibView = mesh.pIndexBuffer->View();
		auto mat = m_model.Materials[mesh.MaterialIndex];
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

		commandList->SetGraphicsRootConstantBufferView(3, m_pMaterialCBs[mesh.MaterialIndex]->GetAddress());

		commandList->SetPipelineState(pso);								// パイプラインステートをセット
		commandList->IASetVertexBuffers(0, 1, &vbView);					// 頂点バッファをスロット0番を使って1個だけ設定する
		commandList->IASetIndexBuffer(&ibView);							// インデックスバッファをセット
		commandList->SetGraphicsRootDescriptorTable(4, mat.pHandle->HandleGPU());	// ディスクリプタテーブルをセット

		commandList->DrawIndexedInstanced(mesh.Indices.size(), 1, 0, 0, 0);
	}

	// アウトラインの描画
	for (Mesh mesh : m_model.Meshes)
	{
		auto vbView = mesh.pVertexBuffer->View();
		auto ibView = mesh.pIndexBuffer->View();
		auto mat = m_model.Materials[mesh.MaterialIndex];
		auto alphaMode = mat.AlphaMode;

		if (alphaMode == 1)
		{
			continue;
		}

		commandList->SetGraphicsRootConstantBufferView(3, m_pMaterialCBs[mesh.MaterialIndex]->GetAddress());

		commandList->SetPipelineState(m_pOutlinePSO->Get());			// パイプラインステートをセット
		commandList->IASetVertexBuffers(0, 1, &vbView);					// 頂点バッファをスロット0番を使って1個だけ設定する
		commandList->IASetIndexBuffer(&ibView);							// インデックスバッファをセットする 
		commandList->SetGraphicsRootDescriptorTable(4, mat.pHandle->HandleGPU());	// ディスクリプタテーブルをセット

		commandList->DrawIndexedInstanced(mesh.Indices.size(), 1, 0, 0, 0);
	}
}

void MeshRenderer::DrawAlpha()
{
	auto currentIndex = g_Engine->CurrentBackBufferIndex();
	auto commandList = g_Engine->CommandList();
	auto materialHeap = m_pDescriptorHeap->GetHeap();

	commandList->SetGraphicsRootSignature(m_pRootSignature->Get());	// ルートシグネチャをセット
	commandList->SetGraphicsRootConstantBufferView(0, m_pTransformCB[currentIndex]->GetAddress());	// 定数バッファをセット
	commandList->SetGraphicsRootConstantBufferView(1, m_pSceneCB[currentIndex]->GetAddress());
	commandList->SetGraphicsRootConstantBufferView(2, m_pBoneCB[currentIndex]->GetAddress());
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	ID3D12DescriptorHeap* heaps[] = {
		m_pDescriptorHeap->GetHeap(),
	};
	commandList->SetDescriptorHeaps(1, heaps);				// ディスクリプタヒープをセット
	commandList->SetGraphicsRootDescriptorTable(5, m_pShadowHandle->HandleGPU());	// ディスクリプタテーブルをセット

	// メッシュの描画
	for (Mesh mesh : m_model.Meshes)
	{
		auto vbView = mesh.pVertexBuffer->View();
		auto ibView = mesh.pIndexBuffer->View();
		auto mat = m_model.Materials[mesh.MaterialIndex];
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

		commandList->SetGraphicsRootConstantBufferView(3, m_pMaterialCBs[mesh.MaterialIndex]->GetAddress());

		commandList->SetPipelineState(pso);								// パイプラインステートをセット
		commandList->IASetVertexBuffers(0, 1, &vbView);					// 頂点バッファをスロット0番を使って1個だけ設定する
		commandList->IASetIndexBuffer(&ibView);							// インデックスバッファをセット
		commandList->SetGraphicsRootDescriptorTable(4, mat.pHandle->HandleGPU());	// ディスクリプタテーブルをセット

		commandList->DrawIndexedInstanced(mesh.Indices.size(), 1, 0, 0, 0);
	}
}

void MeshRenderer::DrawShadow()
{
	auto currentIndex = g_Engine->CurrentBackBufferIndex();
	auto commandList = g_Engine->CommandList();

	commandList->SetGraphicsRootSignature(m_pRootSignature->Get());	// ルートシグネチャをセット
	commandList->SetGraphicsRootConstantBufferView(0, m_pTransformCB[currentIndex]->GetAddress());	// 定数バッファをセット
	commandList->SetGraphicsRootConstantBufferView(1, m_pSceneCB[currentIndex]->GetAddress());
	commandList->SetGraphicsRootConstantBufferView(2, m_pBoneCB[currentIndex]->GetAddress());
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	ID3D12DescriptorHeap* heaps[] = {
		m_pDescriptorHeap->GetHeap(),
	};
	commandList->SetDescriptorHeaps(1, heaps);				// ディスクリプタヒープをセット
	//commandList->SetGraphicsRootDescriptorTable(3, m_pShadowHandle->HandleGPU());	// ディスクリプタテーブルをセット

	// メッシュの描画
	for (Mesh mesh : m_model.Meshes)
	{
		auto vbView = mesh.pVertexBuffer->View();
		auto ibView = mesh.pIndexBuffer->View();
		auto mat = m_model.Materials[mesh.MaterialIndex];

		ID3D12PipelineState* pso = m_pShadowPSO->Get();

		commandList->SetPipelineState(pso);								// パイプラインステートをセット
		commandList->IASetVertexBuffers(0, 1, &vbView);					// 頂点バッファをスロット0番を使って1個だけ設定する
		commandList->IASetIndexBuffer(&ibView);							// インデックスバッファをセットする 
		commandList->SetGraphicsRootDescriptorTable(4, mat.pHandle->HandleGPU());	// ディスクリプタテーブルをセット

		commandList->DrawIndexedInstanced(mesh.Indices.size(), 1, 0, 0, 0);
	}
}

bool MeshRenderer::PreparePSO()
{
	m_pOpaquePSO = new PipelineState();
	m_pOpaquePSO->SetInputLayout(Vertex::InputLayout);
	m_pOpaquePSO->SetRootSignature(m_pRootSignature->Get());
	m_pOpaquePSO->SetVS(L"../x64/Debug/SimpleVS.cso");
	m_pOpaquePSO->SetPS(L"../x64/Debug/SimplePS.cso");
	m_pOpaquePSO->SetCullMode(D3D12_CULL_MODE_FRONT);
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
	m_pShadowPSO->SetCullMode(D3D12_CULL_MODE_BACK);
	m_pShadowPSO->Create();
	if (!m_pShadowPSO->IsValid())
	{
		return false;
	}

}

void MeshRenderer::UpdateBone()
{
	// 行列の更新
	for (auto b : m_model.Bones)
	{
		if (b->GetParent())
		{
			continue;
		}

		b->UpdateMatrices();
	}
}

void MeshRenderer::UpdateCB()
{
	auto currentIndex = g_Engine->CurrentBackBufferIndex();

	// Transform
	auto currentTransform = m_pTransformCB[currentIndex]->GetPtr<TransformParameter>();

	// world行列
	auto scale = transform->scale;
	auto rotation = transform->rotation;
	auto position = transform->position;

	auto world = XMMatrixIdentity();
	world *= XMMatrixScaling(scale.x, scale.y, scale.z);
	world *= XMMatrixRotationQuaternion(XMVectorSet(rotation.x, rotation.y, rotation.z, rotation.w));
	world *= XMMatrixTranslation(position.x, position.y, position.z);

	// view行列
	auto camera = m_pEntity->GetScene()->GetMainCamera();
	auto view = camera->GetViewMatrix();

	// proj行列
	auto proj = camera->GetProjMatrix();

	currentTransform->World = world;
	currentTransform->View = view;
	currentTransform->Proj = proj;

	// Bone
	auto currentBone = m_pBoneCB[currentIndex]->GetPtr<BoneParameter>();

	for (size_t i = 0; i < m_model.Bones.Size(); i++)
	{
		auto bone = m_model.Bones[i];
		auto mtx = bone->GetWorldMatrix() * bone->GetInvBindMatrix();
		XMStoreFloat4x4(&(currentBone->bone[i]), XMMatrixTranspose(mtx));
	}

	// SceneParameter
	auto currentScene = m_pSceneCB[currentIndex]->GetPtr<SceneParameter>();

	auto cameraPos = camera->transform->position;
	currentScene->CameraPosition = cameraPos;
}

Bone* MeshRenderer::FindBone(std::string name)
{
	return m_model.Bones.Find(name);
}
