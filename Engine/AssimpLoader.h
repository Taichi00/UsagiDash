#pragma once
#define NOMINMAX
#include <d3d12.h>
#include <DirectXMath.h>
#include <string>
#include <vector>

struct Model;
struct Mesh;
struct Material;
struct Vertex;
struct CollisionModel;
class Bone;
class BoneList;
class Animation;

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
	AssimpLoader();

	static bool Load(const wchar_t* filename, Model& model, std::vector<Animation*>& animations);	// モデルをロードする
	static bool LoadCollision(const wchar_t* filename, CollisionModel& model);	// コリジョンをロード

private:
	static void ProcessNode(Model& model, aiNode* node, const aiScene* scene);

	static void LoadMesh(Mesh& dst, const aiMesh* src);
	static void LoadMaterial(Material& dst, const aiMaterial* src, const aiScene* scene);
	static void LoadBones(BoneList& bones, Mesh& mesh, const aiMesh* pMesh);
	static void LoadTexture(aiString path, Material& dst, const aiMaterial* src);
	static void LoadEmbeddedTexture(Material& dst, const aiTexture* texture);

	static void BuildBoneHierarchy(BoneList& bones, aiNode* node, Bone* parentBone);

	static void LoadAnimation(Animation* animation, aiAnimation* pAnimation, BoneList* bones);

	static void GenSmoothNormal(Mesh& dst);
	static void GenSmoothNormals(Model& model, aiNode* node, const aiScene* scene);

private:
	
};