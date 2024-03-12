#include "game/entity.h"
#include "game/scene.h"
#include <stdio.h>
#include <typeinfo>

Entity::Entity() : Entity("no_name")
{
}

Entity::Entity(const std::string& name, const std::string& tag, const std::string& layer)
{
	this->name = name;
	this->tag = tag;
	this->layer = layer;

	// transform コンポーネントを追加
	transform = AddComponent<Transform>(new Transform());

	printf("New Entity [%s]\n", name.c_str());
}

Entity::~Entity()
{
	printf("Delete Entity [%s]\n", name.c_str());
}

void Entity::Destroy()
{
	scene_->AddDestroyEntity(this);
}

Component* Entity::AddComponent(Component* component)
{
	component_map_[typeid(*component)].push_back(std::unique_ptr<Component>(component));

	component->RegisterEntity(this);

	return component;
}

void Entity::RegisterScene(Scene* scene)
{
	scene_ = scene;
}

bool Entity::Init()
{
	for (auto& components : component_map_)
	{
		for (auto& component : components.second)
		{
			if (component->enabled)
				component->Init();
		}
	}
	return true;
}

void Entity::BeforeCameraUpdate(const float delta_time)
{
	if (!is_update_enabled_)
		return;

	for (auto& components: component_map_)
	{
		for (auto& component : components.second)
		{
			if (component->enabled)
				component->BeforeCameraUpdate(delta_time);
		}
	}
}

void Entity::CameraUpdate(const float delta_time)
{
	if (!is_update_enabled_)
		return;

	for (auto& components: component_map_)
	{
		for (auto& component : components.second)
		{
			if (component->enabled)
				component->CameraUpdate(delta_time);
		}
	}
}

void Entity::BeforeUpdate(const float delta_time)
{
	if (!is_update_enabled_)
		return;

	for (auto& components : component_map_)
	{
		for (auto& component : components.second)
		{
			if (component->enabled)
				component->BeforeUpdate(delta_time);
		}
	}
}

void Entity::Update(const float delta_time)
{
	if (!is_update_enabled_)
		return;

	for (auto& components: component_map_)
	{
		for (auto& component : components.second)
		{
			if (component->enabled)
				component->Update(delta_time);
		}
	}
}

void Entity::AfterUpdate(const float delta_time)
{
	if (!is_update_enabled_)
		return;

	for (auto& components : component_map_)
	{
		for (auto& component : components.second)
		{
			if (component->enabled)
				component->AfterUpdate(delta_time);
		}
	}
}

void Entity::PhysicsUpdate(const float delta_time)
{
	if (!is_update_enabled_)
		return;

	for (auto& components: component_map_)
	{
		for (auto& component : components.second)
		{
			if (component->enabled)
				component->PhysicsUpdate(delta_time);
		}
	}
}

void Entity::TransformUpdate(const float delta_time)
{
	if (!is_update_enabled_)
		return;

	for (auto& components : component_map_)
	{
		for (auto& component : components.second)
		{
			if (component->enabled)
				component->TransformUpdate(delta_time);
		}
	}
}

void Entity::OnCollisionEnter(Collider* collider)
{
	for (auto& components : component_map_)
	{
		for (auto& component : components.second)
		{
			if (component->enabled)
				component->OnCollisionEnter(collider);
		}
	}
}

void Entity::Draw()
{
	for (auto& components : component_map_)
	{
		for (auto& component : components.second)
		{
			if (component->enabled)
				component->Draw();
		}
	}
}

void Entity::DrawAlpha()
{
	for (auto& components : component_map_)
	{
		for (auto& component : components.second)
		{
			if (component->enabled)
				component->DrawAlpha();
		}
	}
}

void Entity::BeforeDraw()
{
	for (auto& components : component_map_)
	{
		for (auto& component : components.second)
		{
			if (component->enabled)
				component->BeforeDraw();
		}
	}
}

void Entity::DrawShadow()
{
	for (auto& components : component_map_)
	{
		for (auto& component : components.second)
		{
			if (component->enabled)
				component->DrawShadow();
		}
	}
}

void Entity::DrawDepth()
{
	for (auto& components: component_map_)
	{
		for (auto& component : components.second)
		{
			if (component->enabled)
				component->DrawDepth();
		}
	}
}

void Entity::DrawGBuffer()
{
	for (auto& components: component_map_)
	{
		for (auto& component : components.second)
		{
			if (component->enabled)
				component->DrawGBuffer();
		}
	}
}

void Entity::DrawOutline()
{
	for (auto& components: component_map_)
	{
		for (auto& component : components.second)
		{
			if (component->enabled)
				component->DrawOutline();
		}
	}
}

void Entity::Draw2D()
{
	for (auto& components: component_map_)
	{
		for (auto& component : components.second)
		{
			if (component->enabled)
				component->Draw2D();
		}
	}
}

void Entity::OnDestroy()
{
	for (auto& components : component_map_)
	{
		for (auto& component : components.second)
		{
			if (component->enabled)
				component->OnDestroy();
		}
	}
}

void Entity::SetParent(Entity* parent)
{
	if (!parent)
	{
		parent = scene_->RootEntity();
	}

	if (parent_)
	{
		// 元の親からメモリを保持して移動する
		auto ptr = parent_->MoveChild(this);
		parent_->RemoveChild(this);
		parent->AddChild(std::move(ptr));
	}
	else
	{
		parent->AddChild(this);
	}
}

Entity* Entity::Parent()
{
	return parent_;
}

void Entity::AddChild(Entity* child)
{
	child->parent_ = this;
	children_.push_back(std::unique_ptr<Entity>(child));
}

void Entity::AddChild(std::unique_ptr<Entity> child)
{
	child->parent_ = this;
	children_.push_back(std::move(child));
}

void Entity::RemoveChild(Entity* child)
{
	children_.erase(
		std::remove_if(children_.begin(), children_.end(), [&child](const std::unique_ptr<Entity>& e) {
			return e.get() == child;
			}),
		children_.end()
	);
}

std::unique_ptr<Entity> Entity::MoveChild(Entity* child)
{
	std::unique_ptr<Entity> entity = nullptr;

	auto it = std::find_if(children_.begin(), children_.end(),
		[child](const std::unique_ptr<Entity>& ptr) {
			return ptr.get() == child;
		});

	if (it != children_.end())
	{
		auto ptr = std::move(*it);
		children_.erase(it);

		return ptr;
	}

	return nullptr;
}

Entity* Entity::Child(const std::string& name) const
{
	for (const auto& child : children_)
	{
		if (child->name == name)
			return child.get();
	}

	return nullptr;
}

std::vector<Entity*> Entity::Children() const
{
	std::vector<Entity*> children;

	for (const auto& child : children_)
	{
		children.push_back(child.get());
	}

	return children;
}

std::vector<Entity*> Entity::AllChildren() const
{
	std::vector<Entity*> children;

	RecursiveGetChildren(this, children);

	return children;
}

void Entity::RecursiveGetChildren(const Entity* entity, std::vector<Entity*>& list) const
{
	auto children = entity->Children();
	for (const auto& child : children)
	{
		list.push_back(child);
		RecursiveGetChildren(child, list);
	}
}