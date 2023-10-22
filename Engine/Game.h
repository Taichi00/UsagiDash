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
class Scene;

class Game
{
public:
	Game();

	void Run();

	void SetWindowSize(unsigned int width, unsigned int height);
	void SetWindowTitle(std::wstring title);

	Scene* LoadScene(Scene* scene);
	
	/*DirectX::XMMATRIX GetViewMatrix();
	DirectX::XMMATRIX GetProjMatrix();*/

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

	Scene* m_pCurrentScene;
};