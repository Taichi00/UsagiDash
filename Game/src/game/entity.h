#pragma once
#include "game/component/component.h"
#include "game/component/transform.h"
#include "math/vec.h"
#include <unordered_map>
#include <memory>
#include <string>
#include <string>
#include <typeindex>
#include <vector>

class Scene;
class Transform;
class Collider;

class Entity
{
public:
	Entity();
	Entity(
		const std::string& name, 
		const std::string& tag = "untagged", 
		const std::string& layer = "default"
	);

	virtual ~Entity();

	// entity を破棄する
	void Destroy();

	// コンポーネントを追加する
	Component* AddComponent(Component* component);

	template <typename T>
	T* AddComponent(T* component)
	{
		component_map_[typeid(T)].push_back(std::unique_ptr<Component>(component));

		component->RegisterEntity(this);

		return component;
	}

	template <typename T, typename... Args>
	T* AddComponent(Args&&... args)
	{
		auto component = new T(std::forward<Args>(args)...);
		component_map_[typeid(T)].push_back(std::unique_ptr<Component>(component));

		component->RegisterEntity(this);

		return component;
	}
	
	// コンポーネントを取得する
	template<class T>
	T* GetComponent()
	{
		T* component = nullptr;

		for (auto& components : component_map_)
		{
			component = dynamic_cast<T*>(components.second[0].get());
			if (component) break;
		}

		return component;
	}
	
	// シーンを登録する
	void RegisterScene(Scene* scene);
	// シーンを取得する
	Scene* GetScene() const { return scene_; }

	// 更新を止める
	void DisableUpdate() { is_update_enabled_ = false; }
	// 更新を再開する
	void EnableUpdate() { is_update_enabled_ = true; }

	bool IsUpdateEnabled() const { return is_update_enabled_; }

	bool Init();

	void BeforeCameraUpdate(const float delta_time);
	void CameraUpdate(const float delta_time);
	void Update(const float delta_time);
	void PhysicsUpdate(const float delta_time);
	void TransformUpdate(const float delta_time);

	void OnCollisionEnter(Collider* collider);

	void BeforeDraw();
	void DrawShadow();
	void Draw();
	void DrawAlpha();
	void DrawDepth();
	void DrawGBuffer();
	void DrawOutline();
	void Draw2D();

	void OnDestroy();

	// 親エンティティを設定する
	void SetParent(Entity* parent);
	// 子エンティティを追加する
	void AddChild(Entity* child);
	void AddChild(std::unique_ptr<Entity> child);
	// 子エンティティを削除する
	void RemoveChild(Entity* child);
	// 子エンティティの所有権を移動させる
	std::unique_ptr<Entity> MoveChild(Entity* child);

	Entity* Parent();
	Entity* Child(const std::string& name) const;
	std::vector<Entity*> Children() const;
	std::vector<Entity*> AllChildren() const;

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
	std::string name;
	std::string tag;
	std::string layer;

protected:
	// 登録されているシーンへのポインタ
	Scene* scene_ = nullptr;

	// 親エンティティへのポインタ
	Entity* parent_ = nullptr;
	// 子エンティティへのポインタ
	std::vector<std::unique_ptr<Entity>> children_;
	
	// コンポーネントのマップ
	std::unordered_map<std::type_index, std::vector<std::unique_ptr<Component>>> component_map_;

	// 更新を行うかどうか
	bool is_update_enabled_ = true;
};