#include "game/entity.h"
#include <stdio.h>
#include <typeinfo>

Entity::Entity() : Entity("No Name")
{
	
}

Entity::Entity(std::string name)
{
	name_ = name;

	transform = new Transform();
	AddComponent(transform);

	scene_ = nullptr;

	printf("New Entity [%s]\n", name.c_str());
}

Entity::~Entity()
{
	printf("Delete Entity [%s]\n", name_.c_str());
}

Component* Entity::AddComponent(Component* component)
{
	std::string key = typeid(*component).name();

	// すでに同じ型のコンポーネントが存在する場合、追加しない
	if (component_map_.find(key) != component_map_.end())
		return nullptr;

	std::unique_ptr<Component> ucomponent;
	ucomponent.reset(component);
	components_.push_back(std::move(ucomponent));

	component_map_[typeid(*component).name()] = component;

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
	for (auto& component : components_)
	{
		component->Init();
	}
	return true;
}

void Entity::BeforeCameraUpdate()
{
	for (auto& component : components_)
	{
		component->BeforeCameraUpdate();
	}
}

void Entity::CameraUpdate()
{
	for (auto& component : components_)
	{
		component->CameraUpdate();
	}
}

void Entity::Update()
{
	for (auto& component : components_)
	{
		component->Update();
	}
}

void Entity::PhysicsUpdate()
{
	for (auto& component : components_)
	{
		component->PhysicsUpdate();
	}
}

void Entity::DrawDepth()
{
	for (auto& component : components_)
	{
		component->DrawDepth();
	}
}

void Entity::DrawGBuffer()
{
	for (auto& component : components_)
	{
		component->DrawGBuffer();
	}
}

void Entity::DrawOutline()
{
	for (auto& component : components_)
	{
		component->DrawOutline();
	}
}

void Entity::Draw2D()
{
	for (auto& component : components_)
	{
		component->Draw2D();
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

Entity* Entity::GetChild(std::string name)
{
	return children_map_[name];
}

void Entity::Draw()
{
	for (auto& component : components_)
	{
		component->Draw();
	}
}

void Entity::DrawAlpha()
{
	for (auto& component : components_)
	{
		component->DrawAlpha();
	}
}

void Entity::DrawShadow()
{
	for (auto& component : components_)
	{
		component->DrawShadow();
	}
}
