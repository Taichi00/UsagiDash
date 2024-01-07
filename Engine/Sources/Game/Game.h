#pragma once
#include <vector>
#include <string>
#include <DirectXMath.h>
#include <dinput.h>
#include <memory>
#include "ResourceManager.h"

class Window;
class Entity;
class Camera;
class ShadowMap;
class Input;
class Scene;
class ResourceManager;
class Resource;


class Game
{
private:
	Game();
	~Game();

public:
	static Game* Get()
	{
		static Game instance;
		return &instance;
	}

	void Run(Scene* scene);

	void SetWindowSize(unsigned int width, unsigned int height);
	void SetWindowTitle(std::wstring title);

	Scene* LoadScene(Scene* scene);
	Scene* GetCurrentScene();

	DirectX::XMVECTOR GetSWindowSize();

	template<class T> std::shared_ptr<T> LoadResource(std::string path)	// ���\�[�X��ǂݍ���
	{
		return m_pResourceManager->Load<T>(path);
	}
	
	/*DirectX::XMMATRIX GetViewMatrix();
	DirectX::XMMATRIX GetProjMatrix();*/

protected:
	virtual void Init();
	virtual void Update();
	virtual void End();

private:
	std::unique_ptr<Window> m_pWindow;
	unsigned int m_windowWidth = 640;
	unsigned int m_windowHeight = 480;
	std::wstring m_windowTitle = L"Game";

	std::unique_ptr<Scene> m_pCurrentScene;	// ���݂̃V�[���ւ̃|�C���^

	std::unique_ptr<ResourceManager> m_pResourceManager;	// ResourceManager�ւ̃|�C���^
};
