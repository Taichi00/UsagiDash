#pragma once
#include <GLTFSDK/GLTF.h>

struct Mesh;
struct Material;

struct ImportSettings	// インポートするときのパラメータ
{
	const wchar_t* filename = nullptr;	// ファイルパス
	std::vector<Mesh>& meshes;			// 出力先のメッシュ配列
	std::vector<Material>& materials;	// 出力先のマテリアル配列
	bool inverseU = false;				// U座標を反転させるか
	bool inverseV = false;				// V座標を反転させるか
};

class GLTFLoader
{
public:
	bool Load(ImportSettings settings);

private:
	void LoadMesh(const Microsoft::glTF::Document& doc, std::shared_ptr<Microsoft::glTF::GLTFResourceReader> reader);
	void LoadMaterial(const Microsoft::glTF::Document& doc, std::shared_ptr<Microsoft::glTF::GLTFResourceReader> reader);
};