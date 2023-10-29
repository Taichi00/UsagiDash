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

	virtual bool Init();
	virtual void CameraUpdate();
	virtual void Update();
	virtual void PhysicsUpdate();
	virtual void DrawShadow();
	virtual void Draw();
	virtual void DrawAlpha();

public:
	Transform* transform;

protected:
	Scene* m_pScene;
	
	std::vector<Component*> m_components;
	std::map<std::string, Component*> m_componentMap;
};