#include "Core.h"

#include "CollisionManager.h"
#include "ComponentFactory.h"
#include "D3D11Manager.h"
#include "DebugRenderManager.h"
#include "InputManager.h"
#include "LoadManager.h"
#include "PhysicsManager.h"
#include "PrefabManager.h"
#include "RenderManager.h"
#include "Scene.h"
#include "ScriptBehaviourManager.h"
#include "TimeManager.h"
#include "UIManager.h"

engine::Core::~Core()
{
}

void engine::Core::Release()
{
	// TODO : release manager
	PrefabManager::GetInstance().Release();
	m_Scene->Release();
	m_ScriptBehaviourManager->Release();
	m_PhysicsManager->Release();
	m_UIManager->Release();
	ComponentFactory::GetInstance().Release();
	m_CollisionManager->Release();
	m_RenderManager->Release();
	DebugRenderManager::GetInstance().Release();
	LoadManager::GetInstance().Release();
	m_D3D11Manager->Release();
}

void engine::Core::registerObjects()
{
	m_PhysicsManager->RegisterRigidbody();
	m_UIManager->RegisterUI();
	m_RenderManager->RegisterRenderer();
	m_RenderManager->RegisterLight();
	m_CollisionManager->RegisterCollider();
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
	m_CollisionManager->ColliderUpdate();
}

void engine::Core::update()
{
	m_ScriptBehaviourManager->Update();
}

void engine::Core::lateUpdate()
{
	m_ScriptBehaviourManager->LateUpdate();
}

void engine::Core::renderScene()
{
	ComPtr<ID3D11DeviceContext> context = m_D3D11Manager->GetContext();

	m_D3D11Manager->ClearBackBufferView(_float4(0.2f, 0.2f, 0.2f, 1.f));

	m_D3D11Manager->ClearDepthStencilView();

	m_RenderManager->UpdateMainCamera();

	m_RenderManager->Render(context);

	//m_CollisionManager->RenderCollider(context, m_RenderManager->GetViewMat(), m_RenderManager->GetProjMat());

	//m_UIManager->Render(context);

	m_D3D11Manager->Present();
}

void engine::Core::destroy()
{
	m_ScriptBehaviourManager->FlushDestroyScriptBehaviours();
	m_Scene->FlushDestroyGameObjects();
	m_RenderManager->FlushDestroyCamera();
	m_RenderManager->FlushDestroyRenderer();
	m_RenderManager->FlushDestroyLight();
	m_PhysicsManager->FlushDestroyRigidbody();
	m_CollisionManager->FlushDestroyCollider();
	m_UIManager->FlushDestroyUI();
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
	m_CollisionManager 			= &CollisionManager::GetInstance();
	m_RenderManager				= &RenderManager::GetInstance();
	m_UIManager					= &UIManager::GetInstance();

	m_UIManager->Initialize();
	m_TimeManager->Initialize();
	m_InputManager->LockMouse(true);
	m_RenderManager->Initialize(m_D3D11Manager->GetDevice(), m_D3D11Manager->GetContext());

	return S_OK;
}

/*==========================================================
 * Unity의 LifeCycle을 참고해서 Progress을 동작 시키고 있음.
 * 1. Initialization ( TimeUpdate -> register -> Awake -> OnEnable -> Start )
 * 2. Physics ( FixedUpdate -> PhysicsUpdate -> OnTriggerXXX -> OnCollisionXXX )
 * 3. Input Event
 * 4. GameLogic ( Update -> AnimationUpdate -> LateUpdate )
 * 5. Scene Render  ( RenderObject -> OnDrawGizmo -> OnGUI )
 * 6. Decommissioning ( OnDisable -> OnDestroy )
 * ==========================================================*/
void engine::Core::Progress()
{
	Initialization();
	Physics();
	InputEvent();
	GameLogic();
	SceneRender();
	Decommissioning();

	m_Scene->RegisterNextScene();
}

void engine::Core::Initialization()
{
	m_TimeManager->TimeUpdate();
	registerObjects();
	start();
}

void engine::Core::Physics()
{
	fixedUpdate();
	physicsUpdate();
	onTrigger();
	onCollision();
}

void engine::Core::InputEvent()
{
	m_InputManager->UpdateInput(m_Hwnd);
}

void engine::Core::GameLogic()
{
	update();
	lateUpdate();
}


/*=============================================================
 * Rendering 파이프라인 설계
 * 1. PreRender ( SkyBox 등등 )
 * 2. 
 *
 *=============================================================*/
void engine::Core::SceneRender()
{
	renderScene();
}

void engine::Core::Decommissioning()
{
	destroy();
}
