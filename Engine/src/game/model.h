#pragma once

#include "game/bone_list.h"
#include "engine/descriptor_heap.h"
#include "game/resource.h"
#include "engine/shared_struct.h"
#include <DirectXMath.h>
#include <memory>
#include <string>
#include <vector>

class VertexBuffer;
class IndexBuffer;
class Texture2D;
class Animation;

struct Mesh
{
	std::vector<Vertex> vertices;	// ���_�f�[�^�̔z��
	std::vector<uint32_t> indices;	// �C���f�b�N�X�̔z��
	std::shared_ptr<VertexBuffer> vertex_buffer;		// ���_�o�b�t�@�ւ̃|�C���^
	std::shared_ptr<IndexBuffer> index_buffer;		// �C���f�b�N�X�o�b�t�@�ւ̃|�C���^
	int material_index;				// �}�e���A���̃C���f�b�N�X
};

struct Material
{
	std::shared_ptr<Texture2D> albedo_texture;		// �A���x�h�e�N�X�`��
	std::shared_ptr<Texture2D> pbr_texture;			// ���^���b�N�E���t�l�X�e�N�X�`��
	std::shared_ptr<Texture2D> normal_texture;		// �m�[�}���e�N�X�`��
	DescriptorHandle albedo_handle;	// �A���x�h�e�N�X�`���ւ̃f�B�X�N���v�^�n���h��
	DescriptorHandle pbr_handle;		// ���^���b�N�E���t�l�X�e�N�X�`���ւ̃f�B�X�N���v�^�n���h��
	DescriptorHandle normal_handle;	// �m�[�}���e�N�X�`���ւ̃f�B�X�N���v�^�n���h��
	int alpha_mode;					// �������}�e���A�����ǂ���
	XMVECTOR base_color;				// �x�[�X�J���[
};

class Model : public Resource
{
public:
	Model();
	~Model();

	static std::unique_ptr<Model> Load(const std::string& key);
	bool Release() override;

public:
	std::vector<Mesh> meshes;
	std::vector<Material> materials;
	std::vector<std::shared_ptr<Animation>> animations;
	BoneList bones;
	std::shared_ptr<DescriptorHeap> descriptor_heap;
};