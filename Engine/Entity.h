#pragma once
#include "Vec.h"
#include "Component.h"
#include <vector>
#include <string>
#include <map>

class Game;


class Entity
{
public:
	Entity();
	~Entity();
	
	void AddComponent(Component* component);

	template<class T> T* GetComponent()
	{
		return (T*)m_componentMap[typeid(T).name()];
	}

	void RegisterGame(Game* game);
	Game* GetGame();

	virtual bool Init();
	virtual void CameraUpdate();
	virtual void Update();
	virtual void DrawShadow();
	virtual void Draw();
	virtual void DrawAlpha();

	void SetPosition(Vec3 position);
	void SetPosition(float x, float y, float z);

	void SetRotation(Vec3 rotation);
	void SetRotation(float x, float y, float z);

	void SetScale(Vec3 scale);
	void SetScale(float x, float y, float z);

	Vec3 Position();
	Vec3 Rotation();
	Vec3 Scale();

protected:
	Vec3 m_position;
	Vec3 m_rotation;
	Vec3 m_scale;

	Game* m_pGame;
	
	std::vector<Component*> m_components;
	std::map<std::string, Component*> m_componentMap;
};