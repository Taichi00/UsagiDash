#pragma once
#include <GLTFSDK/GLTF.h>

struct Mesh;
struct Material;

struct ImportSettings	// �C���|�[�g����Ƃ��̃p�����[�^
{
	const wchar_t* filename = nullptr;	// �t�@�C���p�X
	std::vector<Mesh>& meshes;			// �o�͐�̃��b�V���z��
	std::vector<Material>& materials;	// �o�͐�̃}�e���A���z��
	bool inverseU = false;				// U���W�𔽓]�����邩
	bool inverseV = false;				// V���W�𔽓]�����邩
};

class GLTFLoader
{
public:
	bool Load(ImportSettings settings);

private:
	void LoadMesh(const Microsoft::glTF::Document& doc, std::shared_ptr<Microsoft::glTF::GLTFResourceReader> reader);
	void LoadMaterial(const Microsoft::glTF::Document& doc, std::shared_ptr<Microsoft::glTF::GLTFResourceReader> reader);
};