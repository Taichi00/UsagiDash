#pragma once
#include <vector>
#include <string>
#include <DirectXMath.h>
#include <memory>
#include "game/resource_manager.h"
#include "game/game_settings.h"
#include "math/vec.h"

class Window;
class Entity;
class Camera;
class ShadowMap;
class Input;
class Scene;
class ResourceManager;
class Resource;
class Engine;
class CollisionManager;
class AudioEngine;
class InputIconManager;

class Game
{
private:
	Game();
	~Game();

	static Game* instance_;

public:
	Game(const Game&) = delete;
	Game& operator=(const Game&) = delete;
	Game(Game&&) = delete;
	Game& operator=(Game&&) = delete;

	static Game* Get()
	{
		return instance_;
	}

	static void Create()
	{
		if (!instance_)
		{
			instance_ = new Game();
		}
	}

	static void Destroy()
	{
		delete instance_;
		instance_ = nullptr;
	}

	void Run(Scene* scene, const GameSettings& settings);

	Engine* GetEngine();
	AudioEngine* GetAudioEngine() { return audio_.get(); }

	void SetWindowSize(unsigned int width, unsigned int height);
	void SetWindowTitle(std::wstring title);

	void ToggleFullscreen();

	Scene* LoadScene(Scene* scene);
	Scene* GetCurrentScene();

	Vec2 GetWindowSize();

	template<class T> std::shared_ptr<T> LoadResource(const std::wstring& path)	// リソースを読み込む
	{
		return resource_manager_->Load<T>(path);
	}

	CollisionManager* GetCollisionManager() { return collision_manager_.get(); }
	ResourceManager* GetResourceManager() { return resource_manager_.get(); }

	double DeltaTime() const;
	
	/*DirectX::XMMATRIX GetViewMatrix();
	DirectX::XMMATRIX GetProjMatrix();*/

protected:
	virtual void Init(const GameSettings& settings);
	virtual void Update();
	virtual void End();

private:
	std::shared_ptr<Window> window_; // ウィンドウへのポインタ
	unsigned int window_width_ = 640;
	unsigned int window_height_ = 480;
	std::wstring window_title_ = L"Game";

	std::unique_ptr<Engine> engine_; // 描画エンジンへのポインタ

	std::unique_ptr<Scene> current_scene_; // 現在のシーンへのポインタ

	std::unique_ptr<ResourceManager> resource_manager_; // ResourceManagerへのポインタ

	std::unique_ptr<CollisionManager> collision_manager_;

	std::unique_ptr<AudioEngine> audio_;

	std::unique_ptr<InputIconManager> input_icon_manager_;

	float delta_time_ = 0.0001;
};