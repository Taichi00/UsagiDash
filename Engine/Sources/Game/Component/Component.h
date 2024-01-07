#pragma once

#include <vector>
#include <typeinfo>

class Entity;
class Transform;

class Component
{
public:
	Component();
	virtual ~Component();

	virtual bool Init();
	virtual void BeforeCameraUpdate();
	virtual void CameraUpdate();
	virtual void Update();
	virtual void PhysicsUpdate();
	virtual void Draw();
	virtual void DrawAlpha();
	virtual void DrawShadow();
	virtual void DrawDepth();
	virtual void DrawGBuffer();
	virtual void DrawOutline();

	void RegisterEntity(Entity* entity);

	/*template<class T> T* GetComponent()
	{
		return (T*)(m_pEntity->GetComponent<T>());
	}*/

	Entity* GetEntity();

public:
	Transform* transform;

private:
	Entity* m_pEntity;

};