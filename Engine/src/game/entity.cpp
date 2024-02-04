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

void Entity::BeforeCameraUpdate()
{
	for (auto& components: component_map_)
	{
		for (auto& component : components.second)
		{
			component->BeforeCameraUpdate();
		}
	}
}

void Entity::CameraUpdate()
{
	for (auto& components: component_map_)
	{
		for (auto& component : components.second)
		{
			component->CameraUpdate();
		}
	}
}

void Entity::Update()
{
	for (auto& components: component_map_)
	{
		for (auto& component : components.second)
		{
			component->Update();
		}
	}
}

void Entity::PhysicsUpdate()
{
	for (auto& components: component_map_)
	{
		for (auto& component : components.second)
		{
			component->PhysicsUpdate();
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
	parent_ = parent;
	parent->children_.push_back(this);
	parent->children_map_[name_] = this;
}

Entity* Entity::GetParent()
{
	return parent_;
}

Entity* Entity::GetChild(const std::string& name)
{
	return children_map_[name];
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
