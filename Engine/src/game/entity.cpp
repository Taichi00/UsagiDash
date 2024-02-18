#include "game/entity.h"
#include "game/scene.h"
#include <stdio.h>
#include <typeinfo>

Entity::Entity() : Entity("No Name")
{
	
}

Entity::Entity(const std::string& name, const std::string& tag)
{
	name_ = name;
	this->tag = tag;

	transform = new Transform();
	AddComponent(transform);

	scene_ = nullptr;

	printf("New Entity [%s]\n", name.c_str());
}

Entity::~Entity()
{
	printf("Delete Entity [%s]\n", name_.c_str());
}

void Entity::Destroy()
{
	scene_->AddDestroyEntity(this);
}

Component* Entity::AddComponent(Component* component)
{
	std::string key = typeid(*component).name();

	std::unique_ptr<Component> ucomponent;
	ucomponent.reset(component);

	component_map_[key].push_back(std::move(ucomponent));

	component->RegisterEntity(this);

	return component;
}


void Entity::RegisterScene(Scene* scene)
{
	scene_ = scene;
}

Scene* Entity::GetScene()
{
	return scene_;
}

bool Entity::Init()
{
	for (auto& components : component_map_)
	{
		for (auto& component : components.second)
		{
			component->Init();
		}
	}
	return true;
}

void Entity::BeforeCameraUpdate(const float delta_time)
{
	for (auto& components: component_map_)
	{
		for (auto& component : components.second)
		{
			component->BeforeCameraUpdate(delta_time);
		}
	}
}

void Entity::CameraUpdate(const float delta_time)
{
	for (auto& components: component_map_)
	{
		for (auto& component : components.second)
		{
			component->CameraUpdate(delta_time);
		}
	}
}

void Entity::Update(const float delta_time)
{
	for (auto& components: component_map_)
	{
		for (auto& component : components.second)
		{
			component->Update(delta_time);
		}
	}
}

void Entity::PhysicsUpdate(const float delta_time)
{
	for (auto& components: component_map_)
	{
		for (auto& component : components.second)
		{
			component->PhysicsUpdate(delta_time);
		}
	}
}

void Entity::OnCollisionEnter(Collider* collider)
{
	for (auto& components : component_map_)
	{
		for (auto& component : components.second)
		{
			component->OnCollisionEnter(collider);
		}
	}
}

void Entity::DrawDepth()
{
	for (auto& components: component_map_)
	{
		for (auto& component : components.second)
		{
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
			component->Draw2D();
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
		// Œ³‚Ìe‚©‚çƒƒ‚ƒŠ‚ð•ÛŽ‚µ‚ÄˆÚ“®‚·‚é
		auto ptr = parent_->MoveChild(this);
		parent_->RemoveChild(this);
		parent->AddChild(std::move(ptr));
	}
	else
	{
		parent->AddChild(this);
	}
}

Entity* Entity::GetParent()
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

Entity* Entity::GetChild(const std::string& name) const
{
	for (const auto& child : children_)
	{
		if (child->name_ == name)
			return child.get();
	}

	return nullptr;
}

std::vector<Entity*> Entity::GetChildren() const
{
	std::vector<Entity*> children;

	for (const auto& child : children_)
	{
		children.push_back(child.get());
	}

	return children;
}

std::vector<Entity*> Entity::GetAllChildren() const
{
	std::vector<Entity*> children;

	RecursiveGetChildren(this, children);

	return children;
}

void Entity::RecursiveGetChildren(const Entity* entity, std::vector<Entity*>& list) const
{
	auto children = entity->GetChildren();
	for (const auto& child : children)
	{
		list.push_back(child);
		RecursiveGetChildren(child, list);
	}
}

void Entity::Draw()
{
	for (auto& components: component_map_)
	{
		for (auto& component : components.second)
		{
			component->Draw();
		}
	}
}

void Entity::DrawAlpha()
{
	for (auto& components: component_map_)
	{
		for (auto& component : components.second)
		{
			component->DrawAlpha();
		}
	}
}

void Entity::DrawShadow()
{
	for (auto& components: component_map_)
	{
		for (auto& component : components.second)
		{
			component->DrawShadow();
		}
	}
}
