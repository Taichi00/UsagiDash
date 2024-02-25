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
	virtual ~Entity();

	void Destroy();
	
	Component* AddComponent(Component* component);

	template<class T>
	T* GetComponent()
	{
		/*auto& components = component_map_[typeid(T).name()];
		if (components.empty())
		{
			return nullptr;
		}*/

		T* component = nullptr;

		for (auto& components : component_map_)
		{
			component = dynamic_cast<T*>(components.second[0].get());
			if (component) break;
		}

		//return (T*)components[0].get();
		return component;
	}

	void RegisterScene(Scene* scene);
	Scene* GetScene();

	bool Init();

	void BeforeCameraUpdate(const float delta_time);
	void CameraUpdate(const float delta_time);
	void Update(const float delta_time);
	void PhysicsUpdate(const float delta_time);
	void TransformUpdate(const float delta_time);

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

	void AddChild(Entity* child);
	void AddChild(std::unique_ptr<Entity> child);
	void RemoveChild(Entity* child);
	std::unique_ptr<Entity> MoveChild(Entity* child);
	Entity* GetChild(const std::string& name) const;
	std::vector<Entity*> GetChildren() const;
	std::vector<Entity*> GetAllChildren() const;

	// 指定したメソッドを再帰的に実行する（親→子）
	template<typename Func>
	void ExecuteOnAllChildren(Func func)
	{
		func(*this);

		for (const auto& child : children_)
		{
			child->ExecuteOnAllChildren(func);
		}
	}

	// 指定したメソッドを再帰的に実行する（子→親）
	template<typename Func>
	void ExecuteOnAllChildrenBack(Func func)
	{
		for (const auto& child : children_)
		{
			child->ExecuteOnAllChildren(func);
		}

		func(*this);
	}

	template<typename Func>
	Entity* FindEntityIf(Func func)
	{
		if (func(*this))
		{
			return this;
		}

		for (const auto& child : children_)
		{
			Entity* entity = child->FindEntityIf(func);
			if (entity)
			{
				return entity;
			}
		}

		return nullptr;
	}

private:
	void RecursiveGetChildren(const Entity* entity, std::vector<Entity*>& list) const;

public:
	Transform* transform;
	std::string tag;

protected:
	Scene* scene_;

	Entity* parent_;								// 親Entity
	std::vector<std::unique_ptr<Entity>> children_;	// 子Entity
	//std::map<std::string, Entity*> children_map_;	// 子Entityの名前マップ
	
	std::map<std::string, std::vector<std::unique_ptr<Component>>> component_map_;

	std::string name_;
};