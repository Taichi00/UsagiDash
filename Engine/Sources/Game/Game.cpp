#include "Game.h"
#include "Engine.h"
#include "Window.h"
#include "Entity.h"
#include "Camera.h"
#include "ShadowMap.h"
#include "Input.h"
#include "Scene.h"
#include "ResourceManager.h"
#include "Resource.h"


Game::Game()
{
}

Game::~Game()
{
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

			if (m_pCurrentScene == nullptr)
				continue;

			m_pCurrentScene->Update();
			m_pCurrentScene->Draw();
		}
	}

	End();
}

void Game::SetWindowSize(unsigned int width, unsigned int height)
{
	m_windowWidth = width;
	m_windowHeight = height;
}

void Game::SetWindowTitle(std::wstring title)
{
	m_windowTitle = title;
}

Scene* Game::LoadScene(Scene* scene)
{
	m_pCurrentScene.reset(scene);
	g_Engine->WaitRender();
	scene->Init();
	return scene;
}

Scene* Game::GetCurrentScene()
{
	return m_pCurrentScene.get();
}

DirectX::XMVECTOR Game::GetSWindowSize()
{
	return { (float)m_windowWidth, (float)m_windowHeight };
}

void Game::Init()
{
	// ウィンドウの生成
	m_pWindow = std::make_unique<Window>(m_windowTitle.c_str(), m_windowWidth, m_windowHeight);

	// 描画エンジンの初期化を行う
	g_Engine = std::make_unique<Engine>();
	if (!g_Engine->Init(m_pWindow.get()))
	{
		return;
	}

	// キー入力
	Input::Create(m_pWindow.get());

	// ResourceManagerの生成
	m_pResourceManager = std::make_unique<ResourceManager>();
}

void Game::Update()
{
}

void Game::End()
{
	Input::Destroy();
}
