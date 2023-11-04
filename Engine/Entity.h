#pragma once
#include "Vec.h"
#include "Component.h"
#include "Transform.h"
#include <vector>
#include <string>
#include <map>
#include <typeinfo>

class Scene;
class Transform;

class Entity
{
public:
	Entity();
	~Entity();
	
	Component* AddComponent(Component* component);

	template<class T> T* GetComponent()
	{
		return (T*)m_componentMap[typeid(T).name()];
	}

	void RegisterScene(Scene* scene);
	Scene* GetScene();

	bool Init();
	void BeforeCameraUpdate();
	void CameraUpdate();
	void Update();
	void PhysicsUpdate();
	void DrawShadow();
	void Draw();
	void DrawAlpha();
	void DrawDepth();
	void DrawGBuffer();
	void DrawOutline();

public:
	Transform* transform;

protected:
	Scene* m_pScene;
	
	std::vector<Component*> m_components;
	std::map<std::string, Component*> m_componentMap;
};