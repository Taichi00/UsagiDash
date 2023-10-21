#pragma once

#include <vector>
#include <typeinfo>

class Entity;
class Transform;

class Component
{
public:
	Component();
	~Component();

	virtual bool Init();
	virtual void CameraUpdate();
	virtual void Update();
	virtual void Draw();
	virtual void DrawAlpha();
	virtual void DrawShadow();

	void RegisterEntity(Entity* entity);

	/*template<class T> T* GetComponent()
	{
		return (T*)(m_pEntity->GetComponent<T>());
	}*/

public:
	Entity* GetEntity();

	Entity* m_pEntity;
	Transform* transform;
};