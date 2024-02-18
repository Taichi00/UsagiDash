#include "game/game.h"
#include "engine/engine.h"
#include "engine/engine2d.h"
#include "engine/window.h"
#include "game/entity.h"
#include "game/component/camera.h"
#include "game/shadow_map.h"
#include "game/input.h"
#include "game/scene.h"
#include "game/resource_manager.h"
#include "game/resource/resource.h"
#include "game/collision_manager.h"
#include "game/audio.h"
#include <chrono>

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

	// 初期シーンを読み込む
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
			delta_time_ = window_->DeltaTime();
			
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
	current_scene_.reset(scene);
	scene->Init();
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

std::shared_ptr<CollisionManager> Game::GetCollisionManager()
{
	return collision_manager_;
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

	// ウィンドウの生成
	window_ = std::make_shared<Window>(window_title_.c_str(), window_width_, window_height_);

	// 描画エンジンの初期化
	engine_ = std::make_unique<Engine>();
	if (!engine_->Init(window_))
	{
		return;
	}

	// オーディオエンジンの初期化
	audio_ = std::make_unique<Audio>();

	// カスタムフォントの読み込み
	engine_->GetEngine2D()->LoadCustomFonts(settings.font_files);

	// キー入力
	Input::Create(window_.get());

	// ResourceManagerの生成
	resource_manager_ = std::make_unique<ResourceManager>();

	// CollisionManagerの生成
	collision_manager_ = std::make_unique<CollisionManager>();
}

void Game::Update()
{
}

void Game::End()
{
	Input::Destroy();
}

