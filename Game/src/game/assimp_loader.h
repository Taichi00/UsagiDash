#pragma once
#define NOMINMAX
#include <d3d12.h>
#include <DirectXMath.h>
#include <string>
#include <vector>
#include <memory>

struct Mesh;
struct Material;
struct Vertex;
class CollisionModel;
class Bone;
class BoneList;
class Animation;
class Texture2D;
class Model;

struct aiMesh;
struct aiMaterial;
struct aiTexture;
struct aiScene;
struct aiString;
struct aiNode;
struct aiAnimation;

class PipelineState;


class AssimpLoader
{
public:
	AssimpLoader() = default;

	static std::unique_ptr<Model> Load(const std::wstring& path);
	static std::unique_ptr<CollisionModel> LoadCollision(const std::wstring& path);

private:
	// �m�[�h�����ǂ��ă��b�V���ƃ{�[����ǂݍ���
	static void ProcessNode(Model& model, aiNode* node, const aiScene* scene);

	// ���b�V����ǂݍ���
	static void LoadMesh(Mesh& dst, const aiMesh* src);

	// �}�e���A����ǂݍ���
	static void LoadMaterial(Material& dst, const aiMaterial* src, const aiScene* scene);

	// �{�[����ǂݍ���
	static void LoadBones(BoneList& bones, Mesh& mesh, const aiMesh* pMesh, aiNode* node);

	// �e�N�X�`����ǂݍ���
	static std::unique_ptr<Texture2D> LoadTexture(
		const aiString& path, 
		const DXGI_FORMAT format,
		const aiMaterial* src, 
		const aiScene* scene
	);

	// ���ߍ��݃e�N�X�`����ǂݍ���
	static std::unique_ptr<Texture2D> LoadEmbeddedTexture(
		const aiTexture* texture,
		const DXGI_FORMAT format
	);

	// bone tree ���\�z����
	static void CreateBoneTree(BoneList& bone_tree, Bone* parent, const aiNode* node);

	static void LoadAnimation(Animation& animation, aiAnimation* src, const BoneList& bones);

	static void GenSmoothNormal(Mesh& dst);

private:
	static std::wstring directory_path;
};