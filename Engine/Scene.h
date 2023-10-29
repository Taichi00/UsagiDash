#pragma once

#include <vector>

class Entity;
class Camera;
class ShadowMap;
class CollisionManager;
class Collider;
class Rigidbody;

class Scene
{
public:
	Scene();
	~Scene();

	virtual bool Init();
	virtual void Update();
	virtual void Draw();

	void CreateEntity(Entity* entity);

	void SetMainCamera(Entity* camera);
	Camera* GetMainCamera();

	ShadowMap* GetShadowMap();

	CollisionManager* GetCollisionManager();

private:
	std::vector<Entity*> m_pEntities;

	Camera* m_pMainCamera = nullptr;
	ShadowMap* m_pShadowMap;
	CollisionManager* m_pCollisionManager;
};