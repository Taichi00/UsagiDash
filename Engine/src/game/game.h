#pragma once
#include <vector>
#include <string>
#include <DirectXMath.h>
#include <memory>
#include "game/resource_manager.h"
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

	void ToggleFullscreen();

	Scene* LoadScene(Scene* scene);
	Scene* GetCurrentScene();

	Vec2 GetWindowSize();

	template<class T> std::shared_ptr<T> LoadResource(const std::string& path)	// ���\�[�X��ǂݍ���
	{
		return resource_manager_->Load<T>(path);
	}

	std::shared_ptr<CollisionManager> GetCollisionManager();
	
	/*DirectX::XMMATRIX GetViewMatrix();
	DirectX::XMMATRIX GetProjMatrix();*/

protected:
	virtual void Init();
	virtual void Update();
	virtual void End();

private:
	std::shared_ptr<Window> window_; // �E�B���h�E�ւ̃|�C���^
	unsigned int window_width_ = 640;
	unsigned int window_height_ = 480;
	std::wstring window_title_ = L"Game";

	std::unique_ptr<Engine> engine_; // �`��G���W���ւ̃|�C���^

	std::unique_ptr<Scene> current_scene_; // ���݂̃V�[���ւ̃|�C���^

	std::unique_ptr<ResourceManager> resource_manager_; // ResourceManager�ւ̃|�C���^

	std::shared_ptr<CollisionManager> collision_manager_;

};
