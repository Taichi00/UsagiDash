#pragma once
#include "Vec.h"
#include "Component.h"
#include "Transform.h"
#include <vector>
#include <string>
#include <map>
#include <typeinfo>
#include <string>
#include <memory>

class Scene;
class Transform;

class Entity
{
public:
	Entity();
	Entity(std::string name);
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

	void SetParent(Entity* parent);
	Entity* GetParent();
	Entity* GetChild(std::string name);

public:
	Transform* transform;

protected:
	Scene* m_pScene;

	Entity* m_pParent;								// 親Entity
	std::vector<Entity*> m_pChildren;				// 子Entity
	std::map<std::string, Entity*> m_pChildrenMap;	// 子Entityの名前マップ
	
	std::vector<std::unique_ptr<Component>> m_components;
	std::map<std::string, Component*> m_componentMap;

	std::string m_name;

};