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
#include "game/layer_manager.h"
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
		// 破棄しないエンティティを避難させる
		dont_destroy_entities = current_scene_->MoveDontDestroyEntities();
	}

	// 新しいシーンの読み込み
	current_scene_.reset(scene);
	scene->Awake();
	scene->Init();
	
	// dont destroy entities を新しいシーンに生成
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

	// ウィンドウの生成
	window_ = std::make_shared<Window>(window_title_.c_str(), window_width_, window_height_);

	// ResourceManagerの生成
	resource_manager_ = std::make_unique<ResourceManager>();

	// 描画エンジンの初期化
	engine_ = std::make_unique<Engine>();
	if (!engine_->Init(window_))
	{
		return;
	}

	// オーディオエンジンの初期化
	audio_ = std::make_unique<AudioEngine>();

	// カスタムフォントの読み込み
	engine_->GetEngine2D()->LoadCustomFonts(settings.font_files);

	// キー入力
	Input::Create(window_.get());

	// LayerManager の生成
	layer_manager_ = std::make_unique<LayerManager>();

	// レイヤーの設定
	for (const auto& name : settings.layers)
	{
		layer_manager_->AddLayer(name);
	}
	for (const auto& info : settings.collision_table)
	{
		layer_manager_->SetCollisionTable(info.layer1, info.layer2, info.collision_enabled);
	}

	// CollisionManagerの生成
	collision_manager_ = std::make_unique<CollisionManager>();

	// キーマップの設定
	for (const auto& action : settings.button_actions)
	{
		Input::Get()->AddButtonAction(action.first, action.second);
	}
	for (const auto& action : settings.axis_actions)
	{
		Input::Get()->AddAxisAction(action.first, action.second);
	}

	// InputIconManager の生成
	input_icon_manager_ = std::make_unique<InputIconManager>();

	for (const auto& action : settings.action_icons)
	{
		input_icon_manager_->AddInputIcon(action.first, action.second);
	}
}

void Game::Update()
{
	// キー入力の更新
	Input::Get()->Update();

	input_icon_manager_->Update();
}

void Game::End()
{
	// シーンを解放
	current_scene_.reset();

	// input を解放
	Input::Destroy();
}

