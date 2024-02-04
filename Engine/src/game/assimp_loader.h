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

	static std::unique_ptr<Model> Load(const std::string& filename);
	static std::unique_ptr<CollisionModel> LoadCollision(const std::string& filename);

private:
	static void ProcessNode(Model& model, aiNode* node, const aiScene* scene);

	static void LoadMesh(Mesh& dst, const aiMesh* src);
	static void LoadMaterial(Material& dst, const aiMaterial* src, const aiScene* scene);
	static void LoadBones(BoneList& bones, Mesh& mesh, const aiMesh* pMesh, aiNode* node);

	static std::unique_ptr<Texture2D> LoadTexture(aiString path, const aiMaterial* src, const aiScene* scene);
	static std::unique_ptr<Texture2D> LoadEmbeddedTexture(const aiTexture* texture);

	static void BuildBoneHierarchy(BoneList& bones, aiNode* node, Bone* parentBone);

	static void LoadAnimation(Animation* animation, aiAnimation* pAnimation, BoneList* bones);

	static void GenSmoothNormal(Mesh& dst);

private:
	
};