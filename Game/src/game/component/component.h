#pragma once

#include <vector>
#include <typeinfo>

class Entity;
class Transform;
class Scene;
class Collider;

class Component
{
public:
	Component();
	virtual ~Component();

	virtual bool Init();

	virtual void BeforeCameraUpdate(const float delta_time) {}
	virtual void CameraUpdate(const float delta_time) {}
	virtual void BeforeUpdate(const float delta_time) {}
	virtual void Update(const float delta_time) {}
	virtual void AfterUpdate(const float delta_time) {}
	virtual void PhysicsUpdate(const float delta_time) {}
	virtual void TransformUpdate(const float delta_time) {}

	virtual void OnCollisionEnter(Collider* collider) {}

	virtual void BeforeDraw() {}
	virtual void Draw() {}
	virtual void DrawAlpha() {}
	virtual void DrawShadow() {}
	virtual void DrawDepth() {}
	virtual void DrawGBuffer() {}
	virtual void DrawOutline() {}
	virtual void Draw2D() {}

	virtual void OnDestroy() {}

	void RegisterEntity(Entity* entity);

	Entity* GetEntity();
	Scene* GetScene();

public:
	Transform* transform;
	bool enabled = true;

private:
	Entity* entity_;

};