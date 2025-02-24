#include "Core.h"

#include "D3D11Manager.h"
#include "InputManager.h"
#include "PhysicsManager.h"
#include "Scene.h"
#include "ScriptBehaviourManager.h"
#include "TimeManager.h"

engine::Core::~Core()
{
	Release();
}

void engine::Core::Release()
{
	// TODO : release manager
	m_D3D11Manager->Release();
	m_Scene->Release();
	m_ScriptBehaviourManager->Release();
	m_PhysicsManager->Release();

}

void engine::Core::start()
{
	m_ScriptBehaviourManager->RegisterScriptBehaviours();
}

void engine::Core::fixedUpdate()
{
	m_ScriptBehaviourManager->FixedUpdate();
}

void engine::Core::physicsUpdate()
{
	m_PhysicsManager->PhysicsUpdate(m_TimeManager->GetDeltaTime());
}

void engine::Core::onTrigger()
{
}

void engine::Core::onCollision()
{
}

void engine::Core::update()
{
	m_ScriptBehaviourManager->Update();
}

void engine::Core::lateUpdate()
{
	m_ScriptBehaviourManager->LateUpdate();
}

void engine::Core::destroy()
{
	m_ScriptBehaviourManager->FlushDestroyScriptBehaviours();
}

HRESULT engine::Core::Initialize(const HWND hwnd)
{
	m_Hwnd = hwnd;

	m_D3D11Manager 				= &D3D11Manager::GetInstance();
	m_Scene 					= &Scene::GetInstance();
	m_ScriptBehaviourManager 	= &ScriptBehaviourManager::GetInstance();
	m_PhysicsManager 			= &PhysicsManager::GetInstance();
	m_TimeManager 				= &TimeManager::GetInstance();
	m_InputManager				= &InputManager::GetInstance();

	m_TimeManager->Initialize();

	return S_OK;
}

void engine::Core::Progress()
{
	m_TimeManager->TimeUpdate();
	start();
	fixedUpdate();
	physicsUpdate();
	onTrigger();
	onCollision();
	m_InputManager->UpdateInput(m_Hwnd);
	update();
	lateUpdate();
	destroy();
}
