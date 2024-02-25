#include "game/game.h"
#include "engine/engine.h"
#include "engine/engine2d.h"
#include "engine/window.h"
#include "game/entity.h"
#include "game/component/camera.h"
#include "game/shadow_map.h"
#include "game/input/input.h"
#include "game/scene.h"
#include "game/resource_manager.h"
#include "game/resource/resource.h"
#include "game/collision_manager.h"
#include "game/audio_engine.h"
#include "game/input/input_icon_manager.h"
#include <chrono>

Game* Game::instance_ = nullptr;

Game::Game()
{
}

Game::~Game()
{
	printf("Delete Game\n");
}

void Game::Run(Scene* scene, const GameSettings& settings)
{
	Init(settings);

	// �����V�[����ǂݍ���
	LoadScene(scene);

	MSG msg = {};
	while (WM_QUIT != msg.message)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE == TRUE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			window_->TickTime();
			delta_time_ = (float)window_->DeltaTime();
			
			Update();

			if (current_scene_)
			{
				//auto start = std::chrono::system_clock::now();
				current_scene_->Update(delta_time_);
				/*auto end = std::chrono::system_clock::now();
				double time = (double)(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() / 1000.0);
				printf("Update:\t%lf[ms]\n", time);*/

				//start = end;
				current_scene_->Draw();
				/*end = std::chrono::system_clock::now();
				time = (double)(std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() / 1000.0);
				printf("Draw: \t%lf[ms]\n", time);*/

				current_scene_->AfterUpdate();
			}
		}
		
	}

	End();
}

Engine* Game::GetEngine()
{
	return engine_.get();
}

void Game::SetWindowSize(unsigned int width, unsigned int height)
{
	window_width_ = width;
	window_height_ = height;
}

void Game::SetWindowTitle(std::wstring title)
{
	window_title_ = title;
}

void Game::ToggleFullscreen()
{
	engine_->ToggleFullscreen();
}

Scene* Game::LoadScene(Scene* scene)
{
	Game::Get()->GetEngine()->WaitGPU();

	std::vector<std::unique_ptr<Entity>> dont_destroy_entities;

	if (current_scene_)
	{
		// �j�����Ȃ��G���e�B�e�B�������
		dont_destroy_entities = current_scene_->MoveDontDestroyEntities();
	}

	// �V�����V�[���̓ǂݍ���
	current_scene_.reset(scene);
	scene->Awake();
	scene->Init();
	
	// dont destroy entities ��V�����V�[���ɐ���
	for (auto& entity : dont_destroy_entities)
	{
		scene->CreateEntity(entity.release());
	}

	return scene;
}

Scene* Game::GetCurrentScene()
{
	return current_scene_.get();
}

Vec2 Game::GetWindowSize()
{
	return Vec2((float)window_->Width(), (float)window_->Height());
}

double Game::DeltaTime() const
{
	return window_->DeltaTime();
}

void Game::Init(const GameSettings& settings)
{
	window_title_ = settings.title;
	window_width_ = settings.window_width;
	window_height_ = settings.window_height;

	// �E�B���h�E�̐���
	window_ = std::make_shared<Window>(window_title_.c_str(), window_width_, window_height_);

	// ResourceManager�̐���
	resource_manager_ = std::make_unique<ResourceManager>();

	// �`��G���W���̏�����
	engine_ = std::make_unique<Engine>();
	if (!engine_->Init(window_))
	{
		return;
	}

	// �I�[�f�B�I�G���W���̏�����
	audio_ = std::make_unique<AudioEngine>();

	// �J�X�^���t�H���g�̓ǂݍ���
	engine_->GetEngine2D()->LoadCustomFonts(settings.font_files);

	// �L�[����
	Input::Create(window_.get());

	// CollisionManager�̐���
	collision_manager_ = std::make_unique<CollisionManager>();

	// �L�[�}�b�v�̐ݒ�
	for (const auto& action : settings.button_actions)
	{
		Input::Get()->AddButtonAction(action.first, action.second);
	}
	for (const auto& action : settings.axis_actions)
	{
		Input::Get()->AddAxisAction(action.first, action.second);
	}

	input_icon_manager_ = std::make_unique<InputIconManager>();

	input_icon_manager_->AddInputIcon("jump", {
		{ Input::InputType::KEYBOARD, L"Assets/image/keyboard_space.png" },
		{ Input::InputType::GAMEPAD, L"Assets/image/xbox_button_color_a.png" }
		});
}

void Game::Update()
{
	// �L�[���͂̍X�V
	Input::Get()->Update();

	input_icon_manager_->Update();
}

void Game::End()
{
	// �V�[�������
	current_scene_.reset();

	// input �����
	Input::Destroy();
}

