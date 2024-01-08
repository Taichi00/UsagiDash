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


MeshRenderer::MeshRenderer(std::shared_ptr<Model> model)
{
	m_pModel = model;
	m_bones = BoneList::Copy(m_pModel->bones);
	m_outlineWidth = 0.003;
}

MeshRenderer::~MeshRenderer()
{
	/*delete[] m_pTransformCB;
	delete[] m_pSceneCB;
	delete[] m_pBoneCB;*/
	delete m_pRootSignature;
	delete m_pOpaquePSO;
	delete m_pAlphaPSO;
	delete m_pOutlinePSO;
	delete m_pShadowPSO;
	delete m_pGBufferPSO;
	delete m_pDepthPSO;
	delete m_pDescriptorHeap;

	/*for (size_t i = 0; i < m_model.Materials.size(); i++)
	{
		auto& mat = m_model.Materials[i];
		delete mat.Texture;
		delete mat.PbrTexture;
		delete mat.NormalTexture;
	}*/
	printf("Delete MeshRenderer\n");
}

void MeshRenderer::SetOutlineWidth(float width)
{
	m_outlineWidth = width;

	for (size_t i = 0; i < m_pModel->materials.size(); i++)
	{
		auto ptr = m_pMaterialCBs[i]->GetPtr<MaterialParameter>();
		ptr->OutlineWidth = m_outlineWidth;
	}
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
	}

	for (size_t i = 0; i < Engine::FRAME_BUFFER_COUNT; i++)
	{
		m_pSceneCB[i] = new ConstantBuffer(sizeof(SceneParameter));
		if (!m_pSceneCB[i]->IsValid())
		{
			printf("ライト用定数バッファの生成に失敗\n");
			return false;
		}
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

	for (size_t i = 0; i < m_pModel->materials.size(); i++)
	{
		ConstantBuffer* cb = new ConstantBuffer(sizeof(MaterialParameter));
		if (!cb->IsValid())
		{
			printf("マテリアル用定数バッファの生成に失敗\n");
			return false;
		}

		auto mat = &m_pModel->materials[i];
		auto ptr = cb->GetPtr<MaterialParameter>();
		ptr->BaseColor = mat->baseColor;
		ptr->OutlineWidth = m_outlineWidth;

		m_pMaterialCBs.push_back(cb);
	}

	// マテリアルの読み込み
	D3D12_DESCRIPTOR_HEAP_DESC desc{};
	desc.NodeMask = 0;	// どのGPU向けのディスクリプタヒープかを指定（GPU１つの場合は０）
	desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	desc.NumDescriptors = m_pModel->materials.size() * 3; // Material数 + ShadowMap
	desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

	m_pDescriptorHeap = new DescriptorHeap(desc);

	m_albedoHandles.resize(m_pModel->materials.size());
	m_pbrHandles.resize(m_pModel->materials.size());
	m_normalHandles.resize(m_pModel->materials.size());

	for (size_t i = 0; i < m_pModel->materials.size(); i++)
	{
		// Albedoテクスチャ
		auto texture = m_pModel->materials[i].albedoTexture.get();
		auto pHandle = m_pDescriptorHeap->Alloc();
		auto resource = texture->Resource();
		auto desc = texture->ViewDesc();
		Engine::Get()->Device()->CreateShaderResourceView(resource, &desc, pHandle.HandleCPU());
		//m_pModel->materials[i].albedoHandle = pHandle;
		m_albedoHandles[i] = pHandle;

		// PBR用のテクスチャ
		texture = m_pModel->materials[i].pbrTexture.get();
		pHandle = m_pDescriptorHeap->Alloc();
		resource = texture->Resource();
		desc = texture->ViewDesc();
		Engine::Get()->Device()->CreateShaderResourceView(resource, &desc, pHandle.HandleCPU());
		//m_pModel->materials[i].pbrHandle = pHandle;
		m_pbrHandles[i] = pHandle;

		// Normalテクスチャ
		texture = m_pModel->materials[i].normalTexture.get();
		pHandle = m_pDescriptorHeap->Alloc();
		resource = texture->Resource();
		desc = texture->ViewDesc();
		Engine::Get()->Device()->CreateShaderResourceView(resource, &desc, pHandle.HandleCPU());
		//m_pModel->materials[i].normalHandle = pHandle;
		m_normalHandles[i] = pHandle;
	}

	// シャドウマップの登録
	//auto pShadowMap = m_pEntity->GetScene()->GetShadowMap();

	/*D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	
	m_pShadowHandle = m_pDescriptorHeap->Alloc();
	Engine::Get()->Device()->CreateShaderResourceView(pShadowMap->Resource(), &srvDesc, m_pShadowHandle->HandleCPU());*/

	RootSignatureParameter params[] = {
		RSConstantBuffer,
		RSConstantBuffer,
		RSConstantBuffer,
		RSConstantBuffer,
		RSTexture,
		RSTexture,
		RSTexture,
	};
	m_pRootSignature = new RootSignature(_countof(params), params);
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
	auto currentIndex = Engine::Get()->CurrentBackBufferIndex();
	auto commandList = Engine::Get()->CommandList();
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
	//commandList->SetGraphicsRootDescriptorTable(5, m_pShadowHandle->HandleGPU());	// ディスクリプタテーブルをセット

	// メッシュの描画
	for (const auto& mesh : m_pModel->meshes)
	{
		auto vbView = mesh.vertexBuffer->View();
		auto ibView = mesh.indexBuffer->View();
		auto materialIndex = mesh.materialIndex;
		auto mat = &m_pModel->materials[mesh.materialIndex];
		auto alphaMode = mat->alphaMode;

		ID3D12PipelineState* pso;
		switch (alphaMode)
		{
		case 1:
			continue;
		default:
			pso = m_pOpaquePSO->Get();
			break;
		}

		commandList->SetGraphicsRootConstantBufferView(3, m_pMaterialCBs[materialIndex]->GetAddress());

		commandList->SetPipelineState(pso);								// パイプラインステートをセット
		commandList->IASetVertexBuffers(0, 1, &vbView);					// 頂点バッファをスロット0番を使って1個だけ設定する
		commandList->IASetIndexBuffer(&ibView);							// インデックスバッファをセット
		commandList->SetGraphicsRootDescriptorTable(4, m_albedoHandles[materialIndex].HandleGPU());	// ディスクリプタテーブルをセット
		commandList->SetGraphicsRootDescriptorTable(5, m_pbrHandles[materialIndex].HandleGPU());

		commandList->DrawIndexedInstanced(mesh.indices.size(), 1, 0, 0, 0);
	}

	// アウトラインの描画
	if (m_outlineWidth <= 0)
		return;

	for (const auto& mesh : m_pModel->meshes)
	{
		auto vbView = mesh.vertexBuffer->View();
		auto ibView = mesh.indexBuffer->View();
		auto materialIndex = mesh.materialIndex;
		auto mat = &m_pModel->materials[mesh.materialIndex];
		auto alphaMode = mat->alphaMode;

		if (alphaMode == 1)
		{
			continue;
		}

		commandList->SetGraphicsRootConstantBufferView(3, m_pMaterialCBs[materialIndex]->GetAddress());

		commandList->SetPipelineState(m_pOutlinePSO->Get());			// パイプラインステートをセット
		commandList->IASetVertexBuffers(0, 1, &vbView);					// 頂点バッファをスロット0番を使って1個だけ設定する
		commandList->IASetIndexBuffer(&ibView);							// インデックスバッファをセットする 
		commandList->SetGraphicsRootDescriptorTable(4, m_albedoHandles[materialIndex].HandleGPU());	// ディスクリプタテーブルをセット

		commandList->DrawIndexedInstanced(mesh.indices.size(), 1, 0, 0, 0);
	}
}

void MeshRenderer::DrawAlpha()
{
	auto currentIndex = Engine::Get()->CurrentBackBufferIndex();
	auto commandList = Engine::Get()->CommandList();
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
	commandList->SetGraphicsRootDescriptorTable(5, m_pShadowHandle.HandleGPU());	// ディスクリプタテーブルをセット

	// メッシュの描画
	for (const auto& mesh : m_pModel->meshes)
	{
		auto vbView = mesh.vertexBuffer->View();
		auto ibView = mesh.indexBuffer->View();
		auto materialIndex = mesh.materialIndex;
		auto mat = &m_pModel->materials[materialIndex];
		auto alphaMode = mat->alphaMode;

		ID3D12PipelineState* pso;
		switch (alphaMode)
		{
		case 1:
			pso = m_pAlphaPSO->Get();
			break;
		default:
			continue;
		}

		commandList->SetGraphicsRootConstantBufferView(3, m_pMaterialCBs[materialIndex]->GetAddress());

		commandList->SetPipelineState(pso);								// パイプラインステートをセット
		commandList->IASetVertexBuffers(0, 1, &vbView);					// 頂点バッファをスロット0番を使って1個だけ設定する
		commandList->IASetIndexBuffer(&ibView);							// インデックスバッファをセット
		commandList->SetGraphicsRootDescriptorTable(4, m_albedoHandles[materialIndex].HandleGPU());	// ディスクリプタテーブルをセット

		commandList->DrawIndexedInstanced(mesh.indices.size(), 1, 0, 0, 0);
	}
}

void MeshRenderer::DrawShadow()
{
	auto currentIndex = Engine::Get()->CurrentBackBufferIndex();
	auto commandList = Engine::Get()->CommandList();
	auto a = m_pTransformCB[currentIndex]->GetPtr<TransformParameter>();
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

	commandList->SetPipelineState(m_pShadowPSO->Get());								// パイプラインステートをセット

	// メッシュの描画
	for (const auto& mesh : m_pModel->meshes)
	{
		auto vbView = mesh.vertexBuffer->View();
		auto ibView = mesh.indexBuffer->View();
		auto materialIndex = mesh.materialIndex;
		auto mat = &m_pModel->materials[materialIndex];

		commandList->IASetVertexBuffers(0, 1, &vbView);					// 頂点バッファをスロット0番を使って1個だけ設定する
		commandList->IASetIndexBuffer(&ibView);							// インデックスバッファをセットする 
		commandList->SetGraphicsRootDescriptorTable(4, m_albedoHandles[materialIndex].HandleGPU());	// ディスクリプタテーブルをセット

		commandList->DrawIndexedInstanced(mesh.indices.size(), 1, 0, 0, 0);
	}
}

void MeshRenderer::DrawDepth()
{
	auto currentIndex = Engine::Get()->CurrentBackBufferIndex();
	auto commandList = Engine::Get()->CommandList();

	commandList->SetGraphicsRootSignature(m_pRootSignature->Get());	// ルートシグネチャをセット
	commandList->SetGraphicsRootConstantBufferView(0, m_pTransformCB[currentIndex]->GetAddress());	// 定数バッファをセット
	commandList->SetGraphicsRootConstantBufferView(2, m_pBoneCB[currentIndex]->GetAddress());
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	ID3D12DescriptorHeap* heaps[] = {
		m_pDescriptorHeap->GetHeap(),
	};
	commandList->SetDescriptorHeaps(1, heaps);				// ディスクリプタヒープをセット

	commandList->SetPipelineState(m_pDepthPSO->Get());								// パイプラインステートをセット

	// メッシュの描画
	for (const auto& mesh : m_pModel->meshes)
	{
		auto vbView = mesh.vertexBuffer->View();
		auto ibView = mesh.indexBuffer->View();
		auto materialIndex = mesh.materialIndex;
		auto mat = &m_pModel->materials[materialIndex];
		auto alphaMode = mat->alphaMode;

		if (alphaMode == 1)
			continue;

		commandList->SetGraphicsRootConstantBufferView(3, m_pMaterialCBs[materialIndex]->GetAddress());
		commandList->IASetVertexBuffers(0, 1, &vbView);					// 頂点バッファをスロット0番を使って1個だけ設定する
		commandList->IASetIndexBuffer(&ibView);							// インデックスバッファをセット
		commandList->SetGraphicsRootDescriptorTable(4, m_albedoHandles[materialIndex].HandleGPU());	// ディスクリプタテーブルをセット

		commandList->DrawIndexedInstanced(mesh.indices.size(), 1, 0, 0, 0);
	}
}

void MeshRenderer::DrawGBuffer()
{
	auto currentIndex = Engine::Get()->CurrentBackBufferIndex();
	auto commandList = Engine::Get()->CommandList();
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

	commandList->SetPipelineState(m_pGBufferPSO->Get());								// パイプラインステートをセット

	// メッシュの描画
	for (const auto& mesh : m_pModel->meshes)
	{
		auto materialIndex = mesh.materialIndex;
		auto vbView = mesh.vertexBuffer->View();
		auto ibView = mesh.indexBuffer->View();
		auto mat = &m_pModel->materials[materialIndex];
		auto alphaMode = mat->alphaMode;

		if (alphaMode == 1)
			continue;

		commandList->SetGraphicsRootConstantBufferView(3, m_pMaterialCBs[materialIndex]->GetAddress());

		commandList->IASetVertexBuffers(0, 1, &vbView);					// 頂点バッファをスロット0番を使って1個だけ設定する
		commandList->IASetIndexBuffer(&ibView);							// インデックスバッファをセット
		commandList->SetGraphicsRootDescriptorTable(4, m_albedoHandles[materialIndex].HandleGPU());	// ディスクリプタテーブルをセット
		commandList->SetGraphicsRootDescriptorTable(5, m_pbrHandles[materialIndex].HandleGPU());
		commandList->SetGraphicsRootDescriptorTable(6, m_normalHandles[materialIndex].HandleGPU());

		commandList->DrawIndexedInstanced(mesh.indices.size(), 1, 0, 0, 0);
	}
}

void MeshRenderer::DrawOutline()
{
	// アウトラインの描画
	if (m_outlineWidth <= 0)
		return;

	auto currentIndex = Engine::Get()->CurrentBackBufferIndex();
	auto commandList = Engine::Get()->CommandList();
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

	commandList->SetPipelineState(m_pOutlinePSO->Get());			// パイプラインステートをセット

	for (const auto& mesh : m_pModel->meshes)
	{
		auto materialIndex = mesh.materialIndex;
		auto vbView = mesh.vertexBuffer->View();
		auto ibView = mesh.indexBuffer->View();
		auto mat = &m_pModel->materials[materialIndex];
		auto alphaMode = mat->alphaMode;

		if (alphaMode == 1)
			continue;

		commandList->SetGraphicsRootConstantBufferView(3, m_pMaterialCBs[materialIndex]->GetAddress());

		commandList->IASetVertexBuffers(0, 1, &vbView);					// 頂点バッファをスロット0番を使って1個だけ設定する
		commandList->IASetIndexBuffer(&ibView);							// インデックスバッファをセットする 
		commandList->SetGraphicsRootDescriptorTable(4, m_albedoHandles[materialIndex].HandleGPU());	// ディスクリプタテーブルをセット

		commandList->DrawIndexedInstanced(mesh.indices.size(), 1, 0, 0, 0);
	}
}

bool MeshRenderer::PreparePSO()
{
	m_pOpaquePSO = new PipelineState();
	m_pOpaquePSO->SetInputLayout(Vertex::InputLayout);
	m_pOpaquePSO->SetRootSignature(m_pRootSignature->Get());
	m_pOpaquePSO->SetVS(L"SimpleVS.cso");
	m_pOpaquePSO->SetPS(L"SimplePS.cso");
	m_pOpaquePSO->SetCullMode(D3D12_CULL_MODE_FRONT);
	m_pOpaquePSO->Create();
	if (!m_pOpaquePSO->IsValid())
	{
		return false;
	}

	m_pAlphaPSO = new PipelineState();
	m_pAlphaPSO->SetInputLayout(Vertex::InputLayout);
	m_pAlphaPSO->SetRootSignature(m_pRootSignature->Get());
	m_pAlphaPSO->SetVS(L"SimpleVS.cso");
	m_pAlphaPSO->SetPS(L"AlphaPS.cso");
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
	m_pOutlinePSO->SetVS(L"OutlineVS.cso");
	m_pOutlinePSO->SetPS(L"OutlinePS.cso");
	m_pOutlinePSO->SetCullMode(D3D12_CULL_MODE_BACK);
	m_pOutlinePSO->Create();
	if (!m_pOutlinePSO->IsValid())
	{
		return false;
	}

	m_pShadowPSO = new PipelineState();
	m_pShadowPSO->SetInputLayout(Vertex::InputLayout);
	m_pShadowPSO->SetRootSignature(m_pRootSignature->Get());
	m_pShadowPSO->SetVS(L"ShadowVS.cso");
	m_pShadowPSO->SetPS(L"ShadowPS.cso");
	m_pShadowPSO->SetCullMode(D3D12_CULL_MODE_FRONT);
	m_pShadowPSO->SetRTVFormat(DXGI_FORMAT_R32G32B32A32_FLOAT);
	m_pShadowPSO->Create();
	if (!m_pShadowPSO->IsValid())
	{
		return false;
	}

	// Depthプリパス用
	m_pDepthPSO = new PipelineState();
	m_pDepthPSO->SetInputLayout(Vertex::InputLayout);
	m_pDepthPSO->SetRootSignature(m_pRootSignature->Get());
	m_pDepthPSO->SetVS(L"SimpleVS.cso");
	m_pDepthPSO->SetPS(L"DepthPS.cso");
	m_pDepthPSO->SetCullMode(D3D12_CULL_MODE_FRONT);
	m_pDepthPSO->SetRTVFormat(DXGI_FORMAT_R8G8B8A8_UNORM);

	/*auto desc = m_pDepthPSO->GetDesc();
	desc->NumRenderTargets = 0;
	desc->RTVFormats[0] = DXGI_FORMAT_UNKNOWN;*/

	m_pDepthPSO->Create();
	if (!m_pDepthPSO->IsValid())
	{
		return false;
	}
	
	// G-Buffer出力用
	m_pGBufferPSO = new PipelineState();
	m_pGBufferPSO->SetInputLayout(Vertex::InputLayout);
	m_pGBufferPSO->SetRootSignature(m_pRootSignature->Get());
	m_pGBufferPSO->SetVS(L"SimpleVS.cso");
	m_pGBufferPSO->SetPS(L"GBufferPS.cso");
	m_pGBufferPSO->SetCullMode(D3D12_CULL_MODE_FRONT);

	auto desc = m_pGBufferPSO->GetDesc();
	desc->NumRenderTargets = 5;
	desc->RTVFormats[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;	// Position
	desc->RTVFormats[1] = DXGI_FORMAT_R32G32B32A32_FLOAT;	// Normal
	desc->RTVFormats[2] = DXGI_FORMAT_R8G8B8A8_UNORM;		// Albedo
	desc->RTVFormats[3] = DXGI_FORMAT_R8G8B8A8_UNORM;		// MetallicRoughness
	desc->RTVFormats[4] = DXGI_FORMAT_R32G32B32A32_FLOAT;	// Depth
	desc->DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;	// デプスバッファには書き込まない
	desc->DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_EQUAL;

	m_pGBufferPSO->Create();
	if (!m_pGBufferPSO->IsValid())
	{
		return false;
	}
}

void MeshRenderer::UpdateBone()
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

void MeshRenderer::UpdateCB()
{
	auto currentIndex = Engine::Get()->CurrentBackBufferIndex();
	auto camera = GetEntity()->GetScene()->GetMainCamera();

	// Transform
	auto currentTransform = m_pTransformCB[currentIndex]->GetPtr<TransformParameter>();
	auto world = transform->GetWorldMatrix();
	auto view = camera->GetViewMatrix();
	auto proj = camera->GetProjMatrix();
	auto ditherLevel = (1.0 - ((transform->position - camera->transform->position).length() - 3) / 3.0) * 16;

	currentTransform->World = world;
	currentTransform->View = view;
	currentTransform->Proj = proj;
	currentTransform->DitherLevel = ditherLevel;

	// Bone
	auto currentBone = m_pBoneCB[currentIndex]->GetPtr<BoneParameter>();

	for (size_t i = 0; i < m_bones.Size(); i++)
	{
		auto bone = m_bones[i];
		auto mtx = bone->GetWorldMatrix() * bone->GetInvBindMatrix();
		XMStoreFloat4x4(&(currentBone->bone[i]), XMMatrixTranspose(mtx));
		XMStoreFloat4x4(&(currentBone->boneNormal[i]), XMMatrixInverse(nullptr, mtx));	// 法線は逆行列の転置で変換
	}

	// SceneParameter
	auto currentScene = m_pSceneCB[currentIndex]->GetPtr<SceneParameter>();
	auto cameraPos = camera->transform->position;
	auto targetPos = camera->GetFocusPosition();
	auto lightPos = targetPos + Vec3(0.5, 3.5, 2.5).normalized() * 500;
	auto lightWorld = world;

	//if (isShadowFromAbove)
	//{
	//	lightWorld = XMMatrixIdentity();
	//	lightWorld *= XMMatrixScalingFromVector(transform->scale);
	//	lightWorld *= XMMatrixRotationQuaternion(transform->rotation);
	//	lightWorld *= XMMatrixRotationQuaternion(Quaternion::FromToRotation((lightPos - targetPos).normalized(), Vec3(0, 1, 0)));
	//	lightWorld *= XMMatrixTranslationFromVector(transform->position);
	//}

	currentScene->CameraPosition = cameraPos;
	currentScene->LightView = XMMatrixLookAtRH(lightPos, targetPos, {0, 1, 0});
	currentScene->LightProj = XMMatrixOrthographicRH(100, 100, 0.1f, 1000.0f);
	currentScene->LightWorld = lightWorld;
}

const Model& MeshRenderer::GetModel() const
{
	return *m_pModel;
}

Bone* MeshRenderer::FindBone(std::string name)
{
	return m_bones.Find(name);
}

BoneList* MeshRenderer::GetBones()
{
	return &(m_bones);
}
