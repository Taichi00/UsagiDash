#pragma once
#define NOMINMAX
#include <d3d12.h>
#include <DirectXMath.h>
#include <string>
#include <vector>

struct Mesh;
struct Material;
struct Vertex;

struct aiMesh;
struct aiMaterial;
struct aiTexture;
struct aiScene;
struct aiString;
struct aiNode;

class PipelineState;

struct ImportSettings	// �C���|�[�g����Ƃ��̃p�����[�^
{
	const wchar_t* filename = nullptr;	// �t�@�C���p�X
	std::vector<Mesh>& meshes;			// �o�͐�̃��b�V���z��
	std::vector<Material>& materials;	// �o�͐�̃}�e���A���z��
	bool inverseU = false;				// U���W�𔽓]�����邩
	bool inverseV = false;				// V���W�𔽓]�����邩
};

class AssimpLoader
{
public:
	bool Load(ImportSettings settings);	// ���f�������[�h����

private:
	void LoadMesh(Mesh& dst, const aiMesh* src, bool inverseU, bool inverseV);
	void LoadMaterial(Material& dst, const aiMaterial* src, const aiScene* scene);
	void LoadBones(Mesh& dst, const aiMesh* src);
	void LoadTexture(aiString path, Material& dst, const aiMaterial* src);
	void LoadEmbeddedTexture(Material& dst, const aiTexture* texture);

};