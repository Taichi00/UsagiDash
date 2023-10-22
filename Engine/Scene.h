#pragma once

#include <vector>

class Entity;
class Camera;
class ShadowMap;

class Scene
{
public:
	Scene();
	~Scene();

	virtual bool Init();
	void Update();
	void Draw();

	void CreateEntity(Entity* entity);

	void SetMainCamera(Entity* camera);
	Camera* GetMainCamera();

	ShadowMap* GetShadowMap();

private:
	std::vector<Entity*> m_pEntities;

	Camera* m_pMainCamera = nullptr;
	ShadowMap* m_pShadowMap;
};