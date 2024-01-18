#include "game/component/mesh_renderer.h"
#include "engine/constant_buffer.h"
#include "engine/descriptor_heap.h"
#include "engine/texture2d.h"
#include "game/entity.h"
#include "game/scene.h"
#include "game/shadow_map.h"
#include "engine/root_signature.h"
#include "engine/pipeline_state.h"
#include "engine/vertex_buffer.h"
#include "engine/index_buffer.h"
#include "game/component/camera.h"
#include "game/bone.h"


MeshRenderer::MeshRenderer()
{
	outline_width_ = 0.003;
}

MeshRenderer::MeshRenderer(std::shared_ptr<Model> model) : MeshRenderer()
{
	model_ = model;
	bones_ = BoneList::Copy(model_->bones);
}

MeshRenderer::~MeshRenderer()
{
	printf("Delete MeshRenderer\n");
}

void MeshRenderer::SetOutlineWidth(float width)
{
	outline_width_ = width;

	for (size_t i = 0; i < model_->materials.size(); i++)
	{
		auto ptr = materials_cb_[i]->GetPtr<MaterialParameter>();
		ptr->outline_width = outline_width_;
	}
}

bool MeshRenderer::Init()
{
	for (size_t i = 0; i < Engine::FRAME_BUFFER_COUNT; i++)
	{
		transform_cb_[i] = Game::Get()->GetEngine()->CreateConstantBuffer(sizeof(TransformParameter));
		scene_cb_[i] = Game::Get()->GetEngine()->CreateConstantBuffer(sizeof(SceneParameter));
		bone_cb_[i] = Game::Get()->GetEngine()->CreateConstantBuffer(sizeof(BoneParameter));
	}

	for (size_t i = 0; i < model_->materials.size(); i++)
	{
		auto cb = Game::Get()->GetEngine()->CreateConstantBuffer(sizeof(MaterialParameter));

		auto& mat = model_->materials[i];
		auto ptr = cb->GetPtr<MaterialParameter>();
		ptr->base_color = mat.base_color;
		ptr->outline_width = outline_width_;

		materials_cb_.push_back(std::move(cb));
	}

	RootSignatureParameter params[] = {
		RSConstantBuffer,
		RSConstantBuffer,
		RSConstantBuffer,
		RSConstantBuffer,
		RSTexture,
		RSTexture,
		RSTexture,
	};
	root_signature_ = std::make_unique<RootSignature>(_countof(params), params);
	if (!root_signature_->IsValid())
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
	auto currentIndex = Game::Get()->GetEngine()->CurrentBackBufferIndex();
	auto commandList = Game::Get()->GetEngine()->CommandList();
	auto materialHeap = model_->descriptor_heap->GetHeap();

	commandList->SetGraphicsRootSignature(root_signature_->Get());	// ルートシグネチャをセット
	commandList->SetGraphicsRootConstantBufferView(0, transform_cb_[currentIndex]->GetAddress());	// 定数バッファをセット
	commandList->SetGraphicsRootConstantBufferView(1, scene_cb_[currentIndex]->GetAddress());
	commandList->SetGraphicsRootConstantBufferView(2, bone_cb_[currentIndex]->GetAddress());
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	
	ID3D12DescriptorHeap* heaps[] = {
		model_->descriptor_heap->GetHeap(),
	};
	commandList->SetDescriptorHeaps(1, heaps);				// ディスクリプタヒープをセット
	//commandList->SetGraphicsRootDescriptorTable(5, m_pShadowHandle->HandleGPU());	// ディスクリプタテーブルをセット

	// メッシュの描画
	for (const auto& mesh : model_->meshes)
	{
		auto vbView = mesh.vertex_buffer->View();
		auto ibView = mesh.index_buffer->View();
		auto materialIndex = mesh.material_index;
		auto mat = &model_->materials[mesh.material_index];
		auto alphaMode = mat->alpha_mode;

		ID3D12PipelineState* pso;
		switch (alphaMode)
		{
		case 1:
			continue;
		default:
			pso = opaque_pso_->Get();
			break;
		}

		commandList->SetGraphicsRootConstantBufferView(3, materials_cb_[materialIndex]->GetAddress());

		commandList->SetPipelineState(pso);								// パイプラインステートをセット
		commandList->IASetVertexBuffers(0, 1, &vbView);					// 頂点バッファをスロット0番を使って1個だけ設定する
		commandList->IASetIndexBuffer(&ibView);							// インデックスバッファをセット
		commandList->SetGraphicsRootDescriptorTable(4, mat->albedo_handle.HandleGPU());	// ディスクリプタテーブルをセット
		commandList->SetGraphicsRootDescriptorTable(5, mat->pbr_handle.HandleGPU());

		commandList->DrawIndexedInstanced(mesh.indices.size(), 1, 0, 0, 0);
	}

	// アウトラインの描画
	if (outline_width_ <= 0)
		return;

	for (const auto& mesh : model_->meshes)
	{
		auto vbView = mesh.vertex_buffer->View();
		auto ibView = mesh.index_buffer->View();
		auto materialIndex = mesh.material_index;
		auto mat = &model_->materials[mesh.material_index];
		auto alphaMode = mat->alpha_mode;

		if (alphaMode == 1)
		{
			continue;
		}

		commandList->SetGraphicsRootConstantBufferView(3, materials_cb_[materialIndex]->GetAddress());

		commandList->SetPipelineState(outline_pso_->Get());			// パイプラインステートをセット
		commandList->IASetVertexBuffers(0, 1, &vbView);					// 頂点バッファをスロット0番を使って1個だけ設定する
		commandList->IASetIndexBuffer(&ibView);							// インデックスバッファをセットする 
		commandList->SetGraphicsRootDescriptorTable(4, mat->albedo_handle.HandleGPU());	// ディスクリプタテーブルをセット

		commandList->DrawIndexedInstanced(mesh.indices.size(), 1, 0, 0, 0);
	}
}

void MeshRenderer::DrawAlpha()
{
	auto currentIndex = Game::Get()->GetEngine()->CurrentBackBufferIndex();
	auto commandList = Game::Get()->GetEngine()->CommandList();
	auto materialHeap = model_->descriptor_heap->GetHeap();

	commandList->SetGraphicsRootSignature(root_signature_->Get());	// ルートシグネチャをセット
	commandList->SetGraphicsRootConstantBufferView(0, transform_cb_[currentIndex]->GetAddress());	// 定数バッファをセット
	commandList->SetGraphicsRootConstantBufferView(1, scene_cb_[currentIndex]->GetAddress());
	commandList->SetGraphicsRootConstantBufferView(2, bone_cb_[currentIndex]->GetAddress());
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	ID3D12DescriptorHeap* heaps[] = {
		model_->descriptor_heap->GetHeap(),
	};
	commandList->SetDescriptorHeaps(1, heaps);				// ディスクリプタヒープをセット
	//commandList->SetGraphicsRootDescriptorTable(5, m_pShadowHandle.HandleGPU());	// ディスクリプタテーブルをセット

	// メッシュの描画
	for (const auto& mesh : model_->meshes)
	{
		auto vbView = mesh.vertex_buffer->View();
		auto ibView = mesh.index_buffer->View();
		auto materialIndex = mesh.material_index;
		auto mat = &model_->materials[materialIndex];
		auto alphaMode = mat->alpha_mode;

		ID3D12PipelineState* pso;
		switch (alphaMode)
		{
		case 1:
			pso = alpha_pso_->Get();
			break;
		default:
			continue;
		}

		commandList->SetGraphicsRootConstantBufferView(3, materials_cb_[materialIndex]->GetAddress());

		commandList->SetPipelineState(pso);								// パイプラインステートをセット
		commandList->IASetVertexBuffers(0, 1, &vbView);					// 頂点バッファをスロット0番を使って1個だけ設定する
		commandList->IASetIndexBuffer(&ibView);							// インデックスバッファをセット
		commandList->SetGraphicsRootDescriptorTable(4, mat->albedo_handle.HandleGPU());	// ディスクリプタテーブルをセット

		commandList->DrawIndexedInstanced(mesh.indices.size(), 1, 0, 0, 0);
	}
}

void MeshRenderer::DrawShadow()
{
	auto currentIndex = Game::Get()->GetEngine()->CurrentBackBufferIndex();
	auto commandList = Game::Get()->GetEngine()->CommandList();
	auto a = transform_cb_[currentIndex]->GetPtr<TransformParameter>();
	commandList->SetGraphicsRootSignature(root_signature_->Get());	// ルートシグネチャをセット
	commandList->SetGraphicsRootConstantBufferView(0, transform_cb_[currentIndex]->GetAddress());	// 定数バッファをセット
	commandList->SetGraphicsRootConstantBufferView(1, scene_cb_[currentIndex]->GetAddress());
	commandList->SetGraphicsRootConstantBufferView(2, bone_cb_[currentIndex]->GetAddress());
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	ID3D12DescriptorHeap* heaps[] = {
		model_->descriptor_heap->GetHeap(),
	};
	commandList->SetDescriptorHeaps(1, heaps);				// ディスクリプタヒープをセット
	//commandList->SetGraphicsRootDescriptorTable(3, m_pShadowHandle->HandleGPU());	// ディスクリプタテーブルをセット

	commandList->SetPipelineState(shadow_pso_->Get());								// パイプラインステートをセット

	// メッシュの描画
	for (const auto& mesh : model_->meshes)
	{
		auto vbView = mesh.vertex_buffer->View();
		auto ibView = mesh.index_buffer->View();
		auto materialIndex = mesh.material_index;
		auto mat = &model_->materials[materialIndex];

		commandList->IASetVertexBuffers(0, 1, &vbView);					// 頂点バッファをスロット0番を使って1個だけ設定する
		commandList->IASetIndexBuffer(&ibView);							// インデックスバッファをセットする 
		commandList->SetGraphicsRootDescriptorTable(4, mat->albedo_handle.HandleGPU());	// ディスクリプタテーブルをセット

		commandList->DrawIndexedInstanced(mesh.indices.size(), 1, 0, 0, 0);
	}
}

void MeshRenderer::DrawDepth()
{
	auto currentIndex = Game::Get()->GetEngine()->CurrentBackBufferIndex();
	auto commandList = Game::Get()->GetEngine()->CommandList();

	commandList->SetGraphicsRootSignature(root_signature_->Get()); // ルートシグネチャをセット
	commandList->SetGraphicsRootConstantBufferView(0, transform_cb_[currentIndex]->GetAddress()); // 定数バッファをセット
	commandList->SetGraphicsRootConstantBufferView(2, bone_cb_[currentIndex]->GetAddress());
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	ID3D12DescriptorHeap* heaps[] = {
		model_->descriptor_heap->GetHeap(),
	};
	commandList->SetDescriptorHeaps(1, heaps); // ディスクリプタヒープをセット

	commandList->SetPipelineState(depth_pso_->Get()); // パイプラインステートをセット

	// メッシュの描画
	for (const auto& mesh : model_->meshes)
	{
		auto vbView = mesh.vertex_buffer->View();
		auto ibView = mesh.index_buffer->View();
		auto materialIndex = mesh.material_index;
		auto mat = &model_->materials[materialIndex];
		auto alphaMode = mat->alpha_mode;

		if (alphaMode == 1)
			continue;

		commandList->SetGraphicsRootConstantBufferView(3, materials_cb_[materialIndex]->GetAddress());
		commandList->IASetVertexBuffers(0, 1, &vbView); // 頂点バッファをスロット0番を使って1個だけ設定する
		commandList->IASetIndexBuffer(&ibView); // インデックスバッファをセット
		commandList->SetGraphicsRootDescriptorTable(4, mat->albedo_handle.HandleGPU()); // ディスクリプタテーブルをセット

		commandList->DrawIndexedInstanced(mesh.indices.size(), 1, 0, 0, 0);
	}
}

void MeshRenderer::DrawGBuffer()
{
	auto currentIndex = Game::Get()->GetEngine()->CurrentBackBufferIndex();
	auto commandList = Game::Get()->GetEngine()->CommandList();
	auto materialHeap = model_->descriptor_heap->GetHeap();

	commandList->SetGraphicsRootSignature(root_signature_->Get());	// ルートシグネチャをセット
	commandList->SetGraphicsRootConstantBufferView(0, transform_cb_[currentIndex]->GetAddress());	// 定数バッファをセット
	commandList->SetGraphicsRootConstantBufferView(1, scene_cb_[currentIndex]->GetAddress());
	commandList->SetGraphicsRootConstantBufferView(2, bone_cb_[currentIndex]->GetAddress());
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	ID3D12DescriptorHeap* heaps[] = {
		model_->descriptor_heap->GetHeap(),
	};
	commandList->SetDescriptorHeaps(1, heaps);				// ディスクリプタヒープをセット

	commandList->SetPipelineState(gbuffer_pso_->Get());								// パイプラインステートをセット

	// メッシュの描画
	for (const auto& mesh : model_->meshes)
	{
		auto materialIndex = mesh.material_index;
		auto vbView = mesh.vertex_buffer->View();
		auto ibView = mesh.index_buffer->View();
		auto mat = &model_->materials[materialIndex];
		auto alphaMode = mat->alpha_mode;

		if (alphaMode == 1)
			continue;

		commandList->SetGraphicsRootConstantBufferView(3, materials_cb_[materialIndex]->GetAddress());

		commandList->IASetVertexBuffers(0, 1, &vbView);					// 頂点バッファをスロット0番を使って1個だけ設定する
		commandList->IASetIndexBuffer(&ibView);							// インデックスバッファをセット
		commandList->SetGraphicsRootDescriptorTable(4, mat->albedo_handle.HandleGPU());	// ディスクリプタテーブルをセット
		commandList->SetGraphicsRootDescriptorTable(5, mat->pbr_handle.HandleGPU());
		commandList->SetGraphicsRootDescriptorTable(6, mat->normal_handle.HandleGPU());

		commandList->DrawIndexedInstanced(mesh.indices.size(), 1, 0, 0, 0);
	}
}

void MeshRenderer::DrawOutline()
{
	// アウトラインの描画
	if (outline_width_ <= 0)
		return;

	auto currentIndex = Game::Get()->GetEngine()->CurrentBackBufferIndex();
	auto commandList = Game::Get()->GetEngine()->CommandList();
	auto materialHeap = model_->descriptor_heap->GetHeap();

	commandList->SetGraphicsRootSignature(root_signature_->Get());	// ルートシグネチャをセット
	commandList->SetGraphicsRootConstantBufferView(0, transform_cb_[currentIndex]->GetAddress());	// 定数バッファをセット
	commandList->SetGraphicsRootConstantBufferView(1, scene_cb_[currentIndex]->GetAddress());
	commandList->SetGraphicsRootConstantBufferView(2, bone_cb_[currentIndex]->GetAddress());
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	ID3D12DescriptorHeap* heaps[] = {
		model_->descriptor_heap->GetHeap(),
	};
	commandList->SetDescriptorHeaps(1, heaps);				// ディスクリプタヒープをセット

	commandList->SetPipelineState(outline_pso_->Get());			// パイプラインステートをセット

	for (const auto& mesh : model_->meshes)
	{
		auto materialIndex = mesh.material_index;
		auto vbView = mesh.vertex_buffer->View();
		auto ibView = mesh.index_buffer->View();
		auto mat = &model_->materials[materialIndex];
		auto alphaMode = mat->alpha_mode;

		if (alphaMode == 1)
			continue;

		commandList->SetGraphicsRootConstantBufferView(3, materials_cb_[materialIndex]->GetAddress());

		commandList->IASetVertexBuffers(0, 1, &vbView);					// 頂点バッファをスロット0番を使って1個だけ設定する
		commandList->IASetIndexBuffer(&ibView);							// インデックスバッファをセットする 
		commandList->SetGraphicsRootDescriptorTable(4, mat->albedo_handle.HandleGPU());	// ディスクリプタテーブルをセット

		commandList->DrawIndexedInstanced(mesh.indices.size(), 1, 0, 0, 0);
	}
}

bool MeshRenderer::PreparePSO()
{
	opaque_pso_ = std::make_unique<PipelineState>();
	opaque_pso_->SetInputLayout(Vertex::InputLayout);
	opaque_pso_->SetRootSignature(root_signature_->Get());
	opaque_pso_->SetVS(L"SimpleVS.cso");
	opaque_pso_->SetPS(L"SimplePS.cso");
	opaque_pso_->SetCullMode(D3D12_CULL_MODE_FRONT);
	opaque_pso_->Create();
	if (!opaque_pso_->IsValid())
	{
		return false;
	}

	alpha_pso_ = std::make_unique<PipelineState>();
	alpha_pso_->SetInputLayout(Vertex::InputLayout);
	alpha_pso_->SetRootSignature(root_signature_->Get());
	alpha_pso_->SetVS(L"SimpleVS.cso");
	alpha_pso_->SetPS(L"AlphaPS.cso");
	alpha_pso_->SetCullMode(D3D12_CULL_MODE_NONE);
	alpha_pso_->SetAlpha();
	alpha_pso_->Create();
	if (!alpha_pso_->IsValid())
	{
		return false;
	}

	outline_pso_ = std::make_unique<PipelineState>();
	outline_pso_->SetInputLayout(Vertex::InputLayout);
	outline_pso_->SetRootSignature(root_signature_->Get());
	outline_pso_->SetVS(L"OutlineVS.cso");
	outline_pso_->SetPS(L"OutlinePS.cso");
	outline_pso_->SetCullMode(D3D12_CULL_MODE_BACK);
	outline_pso_->Create();
	if (!outline_pso_->IsValid())
	{
		return false;
	}

	shadow_pso_ = std::make_unique<PipelineState>();
	shadow_pso_->SetInputLayout(Vertex::InputLayout);
	shadow_pso_->SetRootSignature(root_signature_->Get());
	shadow_pso_->SetVS(L"ShadowVS.cso");
	shadow_pso_->SetPS(L"ShadowPS.cso");
	shadow_pso_->SetCullMode(D3D12_CULL_MODE_FRONT);
	shadow_pso_->SetRTVFormat(DXGI_FORMAT_R32G32B32A32_FLOAT);
	shadow_pso_->Create();
	if (!shadow_pso_->IsValid())
	{
		return false;
	}

	// Depthプリパス用
	depth_pso_ = std::make_unique<PipelineState>();
	depth_pso_->SetInputLayout(Vertex::InputLayout);
	depth_pso_->SetRootSignature(root_signature_->Get());
	depth_pso_->SetVS(L"SimpleVS.cso");
	depth_pso_->SetPS(L"DepthPS.cso");
	depth_pso_->SetCullMode(D3D12_CULL_MODE_FRONT);
	depth_pso_->SetRTVFormat(DXGI_FORMAT_R8G8B8A8_UNORM);

	/*auto desc = m_pDepthPSO->GetDesc();
	desc->NumRenderTargets = 0;
	desc->RTVFormats[0] = DXGI_FORMAT_UNKNOWN;*/

	depth_pso_->Create();
	if (!depth_pso_->IsValid())
	{
		return false;
	}
	
	// G-Buffer出力用
	gbuffer_pso_ = std::make_unique<PipelineState>();
	gbuffer_pso_->SetInputLayout(Vertex::InputLayout);
	gbuffer_pso_->SetRootSignature(root_signature_->Get());
	gbuffer_pso_->SetVS(L"SimpleVS.cso");
	gbuffer_pso_->SetPS(L"GBufferPS.cso");
	gbuffer_pso_->SetCullMode(D3D12_CULL_MODE_FRONT);

	auto desc = gbuffer_pso_->GetDesc();
	desc->NumRenderTargets = 5;
	desc->RTVFormats[0] = DXGI_FORMAT_R32G32B32A32_FLOAT;	// Position
	desc->RTVFormats[1] = DXGI_FORMAT_R32G32B32A32_FLOAT;	// Normal
	desc->RTVFormats[2] = DXGI_FORMAT_R8G8B8A8_UNORM;		// Albedo
	desc->RTVFormats[3] = DXGI_FORMAT_R8G8B8A8_UNORM;		// MetallicRoughness
	desc->RTVFormats[4] = DXGI_FORMAT_R32G32B32A32_FLOAT;	// Depth
	desc->DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;	// デプスバッファには書き込まない
	desc->DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_EQUAL;

	gbuffer_pso_->Create();
	if (!gbuffer_pso_->IsValid())
	{
		return false;
	}
}

void MeshRenderer::UpdateBone()
{
	// 行列の更新
	for (auto b : bones_)
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
	auto currentIndex = Game::Get()->GetEngine()->CurrentBackBufferIndex();
	auto camera = GetEntity()->GetScene()->GetMainCamera();

	// Transform
	auto currentTransform = transform_cb_[currentIndex]->GetPtr<TransformParameter>();
	auto world = transform->GetWorldMatrix();
	auto view = camera->GetViewMatrix();
	auto proj = camera->GetProjMatrix();
	auto ditherLevel = (1.0 - ((transform->position - camera->transform->position).Length() - 3) / 3.0) * 16;

	currentTransform->world = world;
	currentTransform->view = view;
	currentTransform->proj = proj;
	currentTransform->dither_level = ditherLevel;

	// Bone
	auto currentBone = bone_cb_[currentIndex]->GetPtr<BoneParameter>();

	for (size_t i = 0; i < bones_.Size(); i++)
	{
		auto bone = bones_[i];
		auto mtx = bone->GetWorldMatrix() * bone->GetInvBindMatrix();
		XMStoreFloat4x4(&(currentBone->bone[i]), XMMatrixTranspose(mtx));
		XMStoreFloat4x4(&(currentBone->bone_normal[i]), XMMatrixInverse(nullptr, mtx));	// 法線は逆行列の転置で変換
	}

	// SceneParameter
	auto currentScene = scene_cb_[currentIndex]->GetPtr<SceneParameter>();
	auto cameraPos = camera->transform->position;
	auto targetPos = camera->GetFocusPosition();
	auto lightPos = targetPos + Vec3(0.5, 3.5, 2.5).Normalized() * 500;
	auto lightWorld = world;

	//if (isShadowFromAbove)
	//{
	//	lightWorld = XMMatrixIdentity();
	//	lightWorld *= XMMatrixScalingFromVector(transform->scale);
	//	lightWorld *= XMMatrixRotationQuaternion(transform->rotation);
	//	lightWorld *= XMMatrixRotationQuaternion(Quaternion::FromToRotation((lightPos - targetPos).normalized(), Vec3(0, 1, 0)));
	//	lightWorld *= XMMatrixTranslationFromVector(transform->position);
	//}

	currentScene->camera_position = cameraPos;
	currentScene->light_view = XMMatrixLookAtRH(lightPos, targetPos, {0, 1, 0});
	currentScene->light_proj = XMMatrixOrthographicRH(100, 100, 0.1f, 1000.0f);
	currentScene->light_world = lightWorld;
}

const Model& MeshRenderer::GetModel() const
{
	return *model_;
}

Bone* MeshRenderer::FindBone(std::string name)
{
	return bones_.Find(name);
}

BoneList* MeshRenderer::GetBones()
{
	return &(bones_);
}
