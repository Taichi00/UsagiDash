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

			// Camera�̍X�V
			for (auto entity : m_pEntities)
			{
				entity->CameraUpdate();
			}

			// �G���e�B�e�B�̍X�V
			for (auto entity : m_pEntities)
			{
				entity->Update();
			}

			// �����_�����O�̏���
			g_Engine->InitRender();

			// �V���h�E�}�b�v�̕`��
			m_pShadowMap->BeginRender();
			for (auto entity : m_pEntities)
			{
				entity->DrawShadow();
			}
			m_pShadowMap->EndRender();

			// �����_�����O
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
	// �E�B���h�E�̐���
	m_pWindow = new Window(m_windowTitle.c_str(), m_windowWidth, m_windowHeight);

	// �`��G���W���̏��������s��
	g_Engine = new Engine();
	if (!g_Engine->Init(m_pWindow))
	{
		return;
	}

	// �L�[����
	m_pInput = new Input(m_pWindow);

	// �G���e�B�e�B�̃N���A
	m_pEntities.clear();

	// �V���h�E�}�b�v�̐���
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
