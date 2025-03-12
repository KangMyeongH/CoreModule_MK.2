#pragma once
#include "core_defines.h"

namespace engine
{
	class InputManager;
	class TimeManager;
	class PhysicsManager;
	class ScriptBehaviourManager;
	class Scene;
	class D3D11Manager;
	class RenderManager;
	class UIManager;

	class COREMODULE_API Core
	{
	private:
		//======================================//
		//				constructor				//
		//======================================//

		Core() = default;
		~Core();
	public:
		Core(const Core&) = delete;
		Core(Core&&) = delete;
		Core& operator=(const Core&) = delete;
		Core& operator=(Core&&) = delete;

		static Core& GetInstance() { static Core s_Core; return s_Core; }

	public:
		//======================================//
		//				  method				//
		//======================================//

		HRESULT Initialize(HWND hwnd);
		void Progress();
		void Initialization();
		void Physics();
		void InputEvent();
		void GameLogic();
		void SceneRender();
		void Decommissioning();
		void Release();

	private:
		void registerObjects();
		void start();
		void fixedUpdate();
		void physicsUpdate();
		void onTrigger();
		void onCollision();
		void update();
		void lateUpdate();
		void renderScene();
		void destroy();

	private:
		//======================================//
		//				  fields				//
		//======================================//

		HWND					m_Hwnd;
		D3D11Manager* 			m_D3D11Manager;
		TimeManager* 			m_TimeManager;
		InputManager* 			m_InputManager;
		Scene* 					m_Scene;
		ScriptBehaviourManager* m_ScriptBehaviourManager;
		PhysicsManager* 		m_PhysicsManager;
		RenderManager*			m_RenderManager;
		UIManager*				m_UIManager;

		_uint					m_WindowWidth;
		_uint					m_WindowHeight;
	};

}
