#pragma once

#include <vector>

class Entity;

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

public:
	Entity* GetEntity();

protected:
	Entity* m_pEntity;
};