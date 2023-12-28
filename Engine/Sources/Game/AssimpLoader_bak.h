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

struct ImportSettings	// インポートするときのパラメータ
{
	const wchar_t* filename = nullptr;	// ファイルパス
	std::vector<Mesh>& meshes;			// 出力先のメッシュ配列
	std::vector<Material>& materials;	// 出力先のマテリアル配列
	bool inverseU = false;				// U座標を反転させるか
	bool inverseV = false;				// V座標を反転させるか
};

class AssimpLoader
{
public:
	bool Load(ImportSettings settings);	// モデルをロードする

private:
	void LoadMesh(Mesh& dst, const aiMesh* src, bool inverseU, bool inverseV);
	void LoadMaterial(Material& dst, const aiMaterial* src, const aiScene* scene);
	void LoadBones(Mesh& dst, const aiMesh* src);
	void LoadTexture(aiString path, Material& dst, const aiMaterial* src);
	void LoadEmbeddedTexture(Material& dst, const aiTexture* texture);

};