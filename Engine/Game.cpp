#include "Game.h"
#include "Engine.h"
#include "Window.h"
#include "Entity.h"
#include "Camera.h"
#include "ShadowMap.h"
#include "Input.h"

Game::Game()
{
	
}

void Game::Run()
{
	Init();

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

			// Cameraの更新
			for (auto entity : m_pEntities)
			{
				entity->CameraUpdate();
			}

			// エンティティの更新
			for (auto entity : m_pEntities)
			{
				entity->Update();
			}

			// レンダリングの準備
			g_Engine->InitRender();

			// シャドウマップの描画
			m_pShadowMap->BeginRender();
			for (auto entity : m_pEntities)
			{
				entity->DrawShadow();
			}
			m_pShadowMap->EndRender();

			// レンダリング
			g_Engine->BeginRenderMSAA();
			for (auto entity : m_pEntities)
			{
				entity->Draw();
			}
			for (auto entity : m_pEntities)
			{
				entity->DrawAlpha();
			}
			g_Engine->EndRenderMSAA();
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

bool Game::CheckInput(int mode, UINT index)
{
	bool flag = false;

	switch (mode)
	{
	case 0:
		flag = m_pInput->CheckKey(index);
		break;
	case 1:
		flag = m_pInput->TriggerKey(index);
		break;
	default:
		break;
	}

	return flag;
}

void Game::CreateEntity(Entity* entity)
{
	m_pEntities.push_back(entity);
	entity->RegisterGame(this);
	entity->Init();
}

void Game::SetMainCamera(Entity* camera)
{
	m_pMainCamera = camera->GetComponent<Camera>();
}

DirectX::XMMATRIX Game::GetViewMatrix()
{
	return m_pMainCamera->GetViewMatrix();
}

DirectX::XMMATRIX Game::GetProjMatrix()
{
	return m_pMainCamera->GetProjMatrix();
}

ShadowMap* Game::GetShadowMap()
{
	return m_pShadowMap;
}

void Game::Init()
{
	// ウィンドウの生成
	m_pWindow = new Window(m_windowTitle.c_str(), m_windowWidth, m_windowHeight);

	// 描画エンジンの初期化を行う
	g_Engine = new Engine();
	if (!g_Engine->Init(m_pWindow))
	{
		return;
	}

	// キー入力
	m_pInput = new Input(m_pWindow);

	// エンティティのクリア
	m_pEntities.clear();

	// シャドウマップの生成
	m_pShadowMap = new ShadowMap();
}

void Game::Update()
{
}

void Game::End()
{
	delete m_pWindow;
	delete g_Engine;
	delete m_pInput;
	delete m_pShadowMap;
	
	for (auto entity : m_pEntities)
	{
		delete entity;
	}
}
