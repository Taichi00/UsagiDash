#pragma once
#include "Vec.h"
#include "Component.h"
#include "Transform.h"
#include <vector>
#include <string>
#include <map>
#include <typeinfo>

class Game;
class Transform;

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

public:
	Transform* transform;

protected:
	Game* m_pGame;
	
	std::vector<Component*> m_components;
	std::map<std::string, Component*> m_componentMap;
};