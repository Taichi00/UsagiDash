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

	// entity ��j������
	void Destroy();

	// �R���|�[�l���g��ǉ�����
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
	
	// �R���|�[�l���g���擾����
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
	
	// �V�[����o�^����
	void RegisterScene(Scene* scene);
	// �V�[�����擾����
	Scene* GetScene() const { return scene_; }

	// �X�V���~�߂�
	void DisableUpdate() { is_update_enabled_ = false; }
	// �X�V���ĊJ����
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

	// �e�G���e�B�e�B��ݒ肷��
	void SetParent(Entity* parent);
	// �q�G���e�B�e�B��ǉ�����
	void AddChild(Entity* child);
	void AddChild(std::unique_ptr<Entity> child);
	// �q�G���e�B�e�B���폜����
	void RemoveChild(Entity* child);
	// �q�G���e�B�e�B�̏��L�����ړ�������
	std::unique_ptr<Entity> MoveChild(Entity* child);

	Entity* Parent();
	Entity* Child(const std::string& name) const;
	std::vector<Entity*> Children() const;
	std::vector<Entity*> AllChildren() const;

	// �w�肵�����\�b�h���ċA�I�Ɏ��s����i�e���q�j
	template<typename Func>
	void ExecuteOnAllChildren(Func func)
	{
		func(*this);

		for (const auto& child : children_)
		{
			child->ExecuteOnAllChildren(func);
		}
	}

	// �w�肵�����\�b�h���ċA�I�Ɏ��s����i�q���e�j
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
	// �o�^����Ă���V�[���ւ̃|�C���^
	Scene* scene_ = nullptr;

	// �e�G���e�B�e�B�ւ̃|�C���^
	Entity* parent_ = nullptr;
	// �q�G���e�B�e�B�ւ̃|�C���^
	std::vector<std::unique_ptr<Entity>> children_;
	
	// �R���|�[�l���g�̃}�b�v
	std::unordered_map<std::type_index, std::vector<std::unique_ptr<Component>>> component_map_;

	// �X�V���s�����ǂ���
	bool is_update_enabled_ = true;
};