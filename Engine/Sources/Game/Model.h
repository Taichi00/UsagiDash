#pragma once

#include "BoneList.h"
#include "DescriptorHeap.h"
#include "Resource.h"
#include "SharedStruct.h"
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
	std::shared_ptr<VertexBuffer> vertexBuffer;		// ���_�o�b�t�@�ւ̃|�C���^
	std::shared_ptr<IndexBuffer> indexBuffer;		// �C���f�b�N�X�o�b�t�@�ւ̃|�C���^
	int materialIndex;				// �}�e���A���̃C���f�b�N�X
};

struct Material
{
	std::shared_ptr<Texture2D> albedoTexture;		// �A���x�h�e�N�X�`��
	std::shared_ptr<Texture2D> pbrTexture;			// ���^���b�N�E���t�l�X�e�N�X�`��
	std::shared_ptr<Texture2D> normalTexture;		// �m�[�}���e�N�X�`��
	DescriptorHandle albedoHandle;	// �A���x�h�e�N�X�`���ւ̃f�B�X�N���v�^�n���h��
	DescriptorHandle pbrHandle;		// ���^���b�N�E���t�l�X�e�N�X�`���ւ̃f�B�X�N���v�^�n���h��
	DescriptorHandle normalHandle;	// �m�[�}���e�N�X�`���ւ̃f�B�X�N���v�^�n���h��
	int alphaMode;					// �������}�e���A�����ǂ���
	XMVECTOR baseColor;				// �x�[�X�J���[
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
	std::unique_ptr<DescriptorHeap> descriptorHeap;
};