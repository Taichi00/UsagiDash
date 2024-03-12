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
class LayerManager;
class GUIManager;

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
	void Quit();

	// �V�[����ǂݍ���
	Scene* LoadScene(Scene* scene);
	// ���݂̃V�[���ւ̃|�C���^���擾����
	Scene* GetCurrentScene();

	// �E�B���h�E�̑傫����ύX����
	void SetWindowSize(unsigned int width, unsigned int height);
	Vec2 GetWindowSize();
	// �E�B���h�E�̃^�C�g����ύX����
	void SetWindowTitle(std::wstring title);

	// �t���X�N���[���؂�ւ�
	void ToggleFullscreen();

	// ���̓f�o�C�X���ēǂݍ��݂���
	void RestartInput();

	// ���\�[�X��ǂݍ���
	template<class T> std::shared_ptr<T> LoadResource(const std::wstring& path)	
	{
		return resource_manager_->Load<T>(path);
	}

	Engine* GetEngine();
	AudioEngine* GetAudioEngine() { return audio_.get(); }
	CollisionManager* GetCollisionManager() const { return collision_manager_.get(); }
	ResourceManager* GetResourceManager() const { return resource_manager_.get(); }
	LayerManager* GetLayerManager() const { return layer_manager_.get(); }
	GUIManager* GetGUIManager() const { return gui_manager_.get(); }

	double DeltaTime() const;

protected:
	virtual void Init(const GameSettings& settings);
	virtual void Update();
	virtual void End();

	void LoadNextScene();

private:
	std::shared_ptr<Window> window_; // �E�B���h�E�ւ̃|�C���^
	unsigned int window_width_ = 640;
	unsigned int window_height_ = 480;
	std::wstring window_title_ = L"Game";

	std::unique_ptr<Engine> engine_; // �`��G���W���ւ̃|�C���^

	std::unique_ptr<Scene> current_scene_; // ���݂̃V�[���ւ̃|�C���^
	std::unique_ptr<Scene> next_scene_; // ���ɓǂݍ��ރV�[���ւ̃|�C���^

	std::unique_ptr<ResourceManager> resource_manager_; // ResourceManager�ւ̃|�C���^
	std::unique_ptr<CollisionManager> collision_manager_;
	std::unique_ptr<AudioEngine> audio_;
	std::unique_ptr<InputIconManager> input_icon_manager_;
	std::unique_ptr<LayerManager> layer_manager_;
	std::unique_ptr<GUIManager> gui_manager_;

	float delta_time_ = 0.0001f;
};