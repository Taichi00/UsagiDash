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
class Collider;

class Entity
{
public:
	Entity();
	Entity(const std::string& name, const std::string& tag = "untagged");
	~Entity();

	void Destroy();
	
	Component* AddComponent(Component* component);

	template<class T>
	T* GetComponent()
	{
		auto& components = component_map_[typeid(T).name()];
		if (components.empty())
		{
			return nullptr;
		}

		return (T*)components[0].get();
	}

	void RegisterScene(Scene* scene);
	Scene* GetScene();

	bool Init();
	void BeforeCameraUpdate();
	void CameraUpdate();
	void Update();
	void PhysicsUpdate();

	void OnCollisionEnter(Collider* collider);

	void DrawShadow();
	void Draw();
	void DrawAlpha();
	void DrawDepth();
	void DrawGBuffer();
	void DrawOutline();
	void Draw2D();

	void SetParent(Entity* parent);
	Entity* GetParent();
	Entity* GetChild(const std::string& name);

public:
	Transform* transform;
	std::string tag;

protected:
	Scene* scene_;

	Entity* parent_;								// 親Entity
	std::vector<Entity*> children_;					// 子Entity
	std::map<std::string, Entity*> children_map_;	// 子Entityの名前マップ
	
	std::map<std::string, std::vector<std::unique_ptr<Component>>> component_map_;

	std::string name_;
};