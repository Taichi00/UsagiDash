#pragma once
#include <vector>
#include <string>
#include <DirectXMath.h>
#include <memory>
#include "game/resource_manager.h"

class Window;
class Entity;
class Camera;
class ShadowMap;
class Input;
class Scene;
class ResourceManager;
class Resource;
class Engine;


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

	Engine* GetEngine();

	void SetWindowSize(unsigned int width, unsigned int height);
	void SetWindowTitle(std::wstring title);

	Scene* LoadScene(Scene* scene);
	Scene* GetCurrentScene();

	DirectX::XMVECTOR GetSWindowSize();

	template<class T> std::shared_ptr<T> LoadResource(std::string path)	// リソースを読み込む
	{
		return resource_manager_->Load<T>(path);
	}
	
	/*DirectX::XMMATRIX GetViewMatrix();
	DirectX::XMMATRIX GetProjMatrix();*/

protected:
	virtual void Init();
	virtual void Update();
	virtual void End();

private:
	std::unique_ptr<Window> window_; // ウィンドウへのポインタ
	unsigned int window_width_ = 640;
	unsigned int window_height_ = 480;
	std::wstring window_title_ = L"Game";

	std::unique_ptr<Engine> engine_; // 描画エンジンへのポインタ

	std::unique_ptr<Scene> current_scene_; // 現在のシーンへのポインタ

	std::unique_ptr<ResourceManager> resource_manager_; // ResourceManagerへのポインタ
};
