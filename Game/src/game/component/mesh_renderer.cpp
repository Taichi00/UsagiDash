#include "game/component/mesh_renderer.h"
#include "engine/constant_buffer.h"
#include "engine/descriptor_heap.h"
#include "game/resource/texture2d.h"
#include "game/entity.h"
#include "game/scene.h"
#include "game/shadow_map.h"
#include "engine/root_signature.h"
#include "engine/pipeline_state.h"
#include "engine/vertex_buffer.h"
#include "engine/index_buffer.h"
#include "engine/pipeline_state_manager.h"
#include "game/component/camera.h"
#include "game/bone.h"


MeshRenderer::MeshRenderer()
{
	outline_width_ = 0.002f;
}

MeshRenderer::MeshRenderer(std::shared_ptr<Model> model) : MeshRenderer()
{
	model_ = model;
	bones_ = model_->bones.Clone();
}

MeshRenderer::~MeshRenderer()
{
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

	pipeline_manager_ = Game::Get()->GetEngine()->GetPipelineStateManager();

	return true;
}

void MeshRenderer::Update(const float delta_time)
{
	UpdateBone();
}

void MeshRenderer::BeforeDraw()
{
	UpdateCB();
}

void MeshRenderer::Draw()
{
	auto currentIndex = Game::Get()->GetEngine()->CurrentBackBufferIndex();
	auto commandList = Game::Get()->GetEngine()->CommandList();
	auto materialHeap = model_->descriptor_heap->GetHeap();

	commandList->SetGraphicsRootSignature(pipeline_manager_->Get("Opaque")->RootSignature());	// ルートシグネチャをセット
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
			pso = pipeline_manager_->Get("Opaque")->Get();
			break;
		}

		commandList->SetGraphicsRootConstantBufferView(3, materials_cb_[materialIndex]->GetAddress());

		commandList->SetPipelineState(pso);								// パイプラインステートをセット
		commandList->IASetVertexBuffers(0, 1, &vbView);					// 頂点バッファをスロット0番を使って1個だけ設定する
		commandList->IASetIndexBuffer(&ibView);							// インデックスバッファをセット
		commandList->SetGraphicsRootDescriptorTable(4, mat->albedo_handle.HandleGPU());	// ディスクリプタテーブルをセット
		commandList->SetGraphicsRootDescriptorTable(5, mat->pbr_handle.HandleGPU());

		commandList->DrawIndexedInstanced((UINT)mesh.indices.size(), 1, 0, 0, 0);
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

		commandList->SetPipelineState(pipeline_manager_->Get("Outline")->Get());			// パイプラインステートをセット
		commandList->IASetVertexBuffers(0, 1, &vbView);					// 頂点バッファをスロット0番を使って1個だけ設定する
		commandList->IASetIndexBuffer(&ibView);							// インデックスバッファをセットする 
		commandList->SetGraphicsRootDescriptorTable(4, mat->albedo_handle.HandleGPU());	// ディスクリプタテーブルをセット

		commandList->DrawIndexedInstanced((UINT)mesh.indices.size(), 1, 0, 0, 0);
	}
}

void MeshRenderer::DrawAlpha()
{
	auto currentIndex = Game::Get()->GetEngine()->CurrentBackBufferIndex();
	auto commandList = Game::Get()->GetEngine()->CommandList();
	auto materialHeap = model_->descriptor_heap->GetHeap();

	commandList->SetGraphicsRootSignature(pipeline_manager_->Get("Alpha")->RootSignature());	// ルートシグネチャをセット
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
			pso = pipeline_manager_->Get("Alpha")->Get();
			break;
		default:
			continue;
		}

		commandList->SetGraphicsRootConstantBufferView(3, materials_cb_[materialIndex]->GetAddress());

		commandList->SetPipelineState(pso);								// パイプラインステートをセット
		commandList->IASetVertexBuffers(0, 1, &vbView);					// 頂点バッファをスロット0番を使って1個だけ設定する
		commandList->IASetIndexBuffer(&ibView);							// インデックスバッファをセット
		commandList->SetGraphicsRootDescriptorTable(4, mat->albedo_handle.HandleGPU());	// ディスクリプタテーブルをセット

		commandList->DrawIndexedInstanced((UINT)mesh.indices.size(), 1, 0, 0, 0);
	}
}

void MeshRenderer::DrawShadow()
{
	auto currentIndex = Game::Get()->GetEngine()->CurrentBackBufferIndex();
	auto commandList = Game::Get()->GetEngine()->CommandList();
	auto a = transform_cb_[currentIndex]->GetPtr<TransformParameter>();
	commandList->SetGraphicsRootSignature(pipeline_manager_->Get("Shadow")->RootSignature());	// ルートシグネチャをセット
	commandList->SetGraphicsRootConstantBufferView(0, transform_cb_[currentIndex]->GetAddress());	// 定数バッファをセット
	commandList->SetGraphicsRootConstantBufferView(1, scene_cb_[currentIndex]->GetAddress());
	commandList->SetGraphicsRootConstantBufferView(2, bone_cb_[currentIndex]->GetAddress());
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	ID3D12DescriptorHeap* heaps[] = {
		model_->descriptor_heap->GetHeap(),
	};
	commandList->SetDescriptorHeaps(1, heaps);				// ディスクリプタヒープをセット
	//commandList->SetGraphicsRootDescriptorTable(3, m_pShadowHandle->HandleGPU());	// ディスクリプタテーブルをセット

	commandList->SetPipelineState(pipeline_manager_->Get("Shadow")->Get());								// パイプラインステートをセット

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

		commandList->DrawIndexedInstanced((UINT)mesh.indices.size(), 1, 0, 0, 0);
	}
}

void MeshRenderer::DrawDepth()
{
	auto currentIndex = Game::Get()->GetEngine()->CurrentBackBufferIndex();
	auto commandList = Game::Get()->GetEngine()->CommandList();

	commandList->SetGraphicsRootSignature(pipeline_manager_->Get("Depth")->RootSignature()); // ルートシグネチャをセット
	commandList->SetGraphicsRootConstantBufferView(0, transform_cb_[currentIndex]->GetAddress()); // 定数バッファをセット
	commandList->SetGraphicsRootConstantBufferView(2, bone_cb_[currentIndex]->GetAddress());
	ID3D12DescriptorHeap* heaps[] = {
		model_->descriptor_heap->GetHeap(),
	};
	commandList->SetDescriptorHeaps(1, heaps); // ディスクリプタヒープをセット

	commandList->SetPipelineState(pipeline_manager_->Get("Depth")->Get()); // パイプラインステートをセット

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

		commandList->DrawIndexedInstanced((UINT)mesh.indices.size(), 1, 0, 0, 0);
	}
}

void MeshRenderer::DrawGBuffer()
{
	auto currentIndex = Game::Get()->GetEngine()->CurrentBackBufferIndex();
	auto commandList = Game::Get()->GetEngine()->CommandList();
	auto materialHeap = model_->descriptor_heap->GetHeap();

	commandList->SetGraphicsRootSignature(pipeline_manager_->Get("GBuffer")->RootSignature()); // ルートシグネチャをセット
	commandList->SetGraphicsRootConstantBufferView(0, transform_cb_[currentIndex]->GetAddress()); // 定数バッファをセット
	commandList->SetGraphicsRootConstantBufferView(1, scene_cb_[currentIndex]->GetAddress());
	commandList->SetGraphicsRootConstantBufferView(2, bone_cb_[currentIndex]->GetAddress());
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	ID3D12DescriptorHeap* heaps[] = {
		model_->descriptor_heap->GetHeap(),
	};
	commandList->SetDescriptorHeaps(1, heaps); // ディスクリプタヒープをセット

	commandList->SetPipelineState(pipeline_manager_->Get("GBuffer")->Get()); // パイプラインステートをセット

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

		commandList->IASetVertexBuffers(0, 1, &vbView); // 頂点バッファをスロット0番を使って1個だけ設定する
		commandList->IASetIndexBuffer(&ibView); // インデックスバッファをセット
		commandList->SetGraphicsRootDescriptorTable(4, mat->albedo_handle.HandleGPU()); // ディスクリプタテーブルをセット
		commandList->SetGraphicsRootDescriptorTable(5, mat->pbr_handle.HandleGPU());
		commandList->SetGraphicsRootDescriptorTable(6, mat->normal_handle.HandleGPU());

		commandList->DrawIndexedInstanced((UINT)mesh.indices.size(), 1, 0, 0, 0);
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

	commandList->SetGraphicsRootSignature(pipeline_manager_->Get("Outline")->RootSignature()); // ルートシグネチャをセット
	commandList->SetGraphicsRootConstantBufferView(0, transform_cb_[currentIndex]->GetAddress()); // 定数バッファをセット
	commandList->SetGraphicsRootConstantBufferView(1, scene_cb_[currentIndex]->GetAddress());
	commandList->SetGraphicsRootConstantBufferView(2, bone_cb_[currentIndex]->GetAddress());
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	ID3D12DescriptorHeap* heaps[] = {
		model_->descriptor_heap->GetHeap(),
	};
	commandList->SetDescriptorHeaps(1, heaps); // ディスクリプタヒープをセット

	commandList->SetPipelineState(pipeline_manager_->Get("Outline")->Get()); // パイプラインステートをセット

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

		commandList->IASetVertexBuffers(0, 1, &vbView); // 頂点バッファをスロット0番を使って1個だけ設定する
		commandList->IASetIndexBuffer(&ibView); // インデックスバッファをセットする 
		commandList->SetGraphicsRootDescriptorTable(4, mat->albedo_handle.HandleGPU()); // ディスクリプタテーブルをセット

		commandList->DrawIndexedInstanced((UINT)mesh.indices.size(), 1, 0, 0, 0);
	}
}

void MeshRenderer::UpdateBone()
{
	auto root = bones_.RootBone();

	// 行列の更新
	if (root)
		root->UpdateMatrices();
}

void MeshRenderer::UpdateCB()
{
	auto current_index = Game::Get()->GetEngine()->CurrentBackBufferIndex();
	auto camera = GetEntity()->GetScene()->GetMainCamera();

	auto aabb = model_->aabb;
	aabb.Scale(transform->scale);
	aabb.Translate(transform->WorldPosition());

	auto center = aabb.Center();
	float radius = aabb.Size().Length() / 2;

	// Transform
	auto current_transform = transform_cb_[current_index]->GetPtr<TransformParameter>();
	auto world = transform->WorldMatrix();
	auto view = camera->GetViewMatrix();
	auto proj = camera->GetProjMatrix();
	auto dither_level = (1.0f - std::max((center - camera->Position()).Length() - radius, 0.f) / 3.0f) * 16;
	dither_level = std::max(dither_level, dither_level_ * 16);

	current_transform->world = world;
	current_transform->view = view;
	current_transform->proj = proj;
	current_transform->dither_level = dither_level;

	// Bone
	auto current_bone = bone_cb_[current_index]->GetPtr<BoneParameter>();

	for (int i = 0; i < bones_.Size(); i++)
	{
		auto bone = bones_[i];
		auto mtx = bone->WorldMatrix() * bone->InvBindMatrix();
		XMStoreFloat4x4(&(current_bone->bone[i]), XMMatrixTranspose(mtx));
		XMStoreFloat4x4(&(current_bone->bone_normal[i]), XMMatrixInverse(nullptr, mtx)); // 法線は逆行列の転置で変換
	}

	// Material
	for (int i = 0; i < model_->materials.size(); i++)
	{
		auto ptr = materials_cb_[i]->GetPtr<MaterialParameter>();
		ptr->outline_width = outline_width_;
	}

	// SceneParameter
	auto current_scene = scene_cb_[current_index]->GetPtr<SceneParameter>();
	auto camera_pos = camera->Position();
	auto target_pos = camera->GetFocusPosition();
	auto light_pos = target_pos + Vec3(0.5, 3.5, 2.5).Normalized() * 500;
	auto light_world = world;

	current_scene->camera_position = camera_pos;
	current_scene->light_view = XMMatrixLookAtRH(light_pos, target_pos, {0, 1, 0});
	current_scene->light_proj = XMMatrixOrthographicRH(100, 100, 0.1f, 1000.0f);
	current_scene->light_world = light_world;
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
	return &bones_;
}
