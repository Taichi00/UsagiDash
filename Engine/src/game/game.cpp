#include "game/game.h"
#include "engine/engine.h"
#include "engine/window.h"
#include "game/entity.h"
#include "game/component/camera.h"
#include "game/shadow_map.h"
#include "game/input.h"
#include "game/scene.h"
#include "game/resource_manager.h"
#include "game/resource.h"


Game::Game()
{
}

Game::~Game()
{
	printf("Delete Game\n");
}

void Game::Run(Scene* scene)
{
	Init();

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
			Update();

			if (current_scene_ == nullptr)
				continue;

			current_scene_->Update();
			current_scene_->Draw();
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

Scene* Game::LoadScene(Scene* scene)
{
	Game::Get()->GetEngine()->WaitRender();
	current_scene_.reset(scene);
	scene->Init();
	return scene;
}

Scene* Game::GetCurrentScene()
{
	return current_scene_.get();
}

DirectX::XMVECTOR Game::GetSWindowSize()
{
	return { (float)window_width_, (float)window_height_ };
}

void Game::Init()
{
	// ウィンドウの生成
	window_ = std::make_unique<Window>(window_title_.c_str(), window_width_, window_height_);

	// 描画エンジンの初期化を行う
	engine_ = std::make_unique<Engine>();
	if (!Game::Get()->GetEngine()->Init(window_.get()))
	{
		return;
	}

	// キー入力
	Input::Create(window_.get());

	// ResourceManagerの生成
	resource_manager_ = std::make_unique<ResourceManager>();
}

void Game::Update()
{
}

void Game::End()
{
	Input::Destroy();
}

