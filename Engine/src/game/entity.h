#pragma once
#include "math/vec.h"
#include "game/component/component.h"
#include "game/component/transform.h"
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
		return (T*)component_map_[typeid(T).name()];
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
	void Draw2D();

	void SetParent(Entity* parent);
	Entity* GetParent();
	Entity* GetChild(std::string name);

public:
	Transform* transform;

protected:
	Scene* scene_;

	Entity* parent_;								// 親Entity
	std::vector<Entity*> children_;					// 子Entity
	std::map<std::string, Entity*> children_map_;	// 子Entityの名前マップ
	
	std::vector<std::unique_ptr<Component>> components_;
	std::map<std::string, Component*> component_map_;

	std::string name_;

};