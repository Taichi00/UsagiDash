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
	virtual void Update(const float delta_time) {}
	virtual void PhysicsUpdate(const float delta_time) {}
	virtual void TransformUpdate(const float delta_time) {}

	virtual void OnCollisionEnter(Collider* collider) {}

	virtual void Draw() {}
	virtual void DrawAlpha() {}
	virtual void DrawShadow() {}
	virtual void DrawDepth() {}
	virtual void DrawGBuffer() {}
	virtual void DrawOutline() {}
	virtual void Draw2D() {}

	void RegisterEntity(Entity* entity);

	/*template<class T> T* GetComponent()
	{
		return (T*)(m_pEntity->GetComponent<T>());
	}*/

	Entity* GetEntity();
	Scene* GetScene();

public:
	Transform* transform;

private:
	Entity* entity_;

};