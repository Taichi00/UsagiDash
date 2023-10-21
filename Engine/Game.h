#pragma once
#include <vector>
#include <string>
#include <DirectXMath.h>
#include <dinput.h>

class Window;
class Entity;
class Camera;
class ShadowMap;
class Input;

class Game
{
public:
	Game();

	void Run();

	void SetWindowSize(unsigned int width, unsigned int height);
	void SetWindowTitle(std::wstring title);

	void CreateEntity(Entity* entity);

	void SetMainCamera(Entity* camera);
	Camera* GetMainCamera();
	DirectX::XMMATRIX GetViewMatrix();
	DirectX::XMMATRIX GetProjMatrix();

	ShadowMap* GetShadowMap();

protected:
	virtual void Init();
	virtual void Update();
	virtual void End();

private:
	Window* m_pWindow;
	unsigned int m_windowWidth = 640;
	unsigned int m_windowHeight = 480;
	std::wstring m_windowTitle = L"Game";

	Input* m_pInput;
	
	std::vector<Entity*> m_pEntities;

	Camera* m_pMainCamera = nullptr;

	ShadowMap* m_pShadowMap;
};