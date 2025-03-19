#include "ScriptBehaviourManager.h"

#include "ScriptBehaviour.h"

IMPLEMENT_SINGLETON(engine::ScriptBehaviourManager)

engine::ScriptBehaviourManager::~ScriptBehaviourManager()
{

}

void engine::ScriptBehaviourManager::FixedUpdate() const
{
	for (const auto& scriptBehaviour : m_ScriptBehaviours)
	{
		if (scriptBehaviour->IsEnabled())
		{
			scriptBehaviour->FixedUpdate();
		}
	}
}

void engine::ScriptBehaviourManager::Update() const
{
	for (const auto& scriptBehaviour : m_ScriptBehaviours)
	{
		if (scriptBehaviour->IsEnabled())
		{
			scriptBehaviour->Update();
		}
	}
}

void engine::ScriptBehaviourManager::LateUpdate() const
{
	for (const auto& scriptBehaviour : m_ScriptBehaviours)
	{
		if (scriptBehaviour->IsEnabled())
		{
			scriptBehaviour->LateUpdate();
		}
	}
}

void engine::ScriptBehaviourManager::AddScriptBehaviour(const SharedPtr<ScriptBehaviour>& scriptBehaviour)
{
	m_RegisterQueue.push_back(scriptBehaviour);
	scriptBehaviour->Awake();
}

void engine::ScriptBehaviourManager::RegisterScriptBehaviours()
{
	for (auto it = m_RegisterQueue.begin(); it != m_RegisterQueue.end();)
	{
		SharedPtr<ScriptBehaviour> scriptBehaviour = *it;

		if (scriptBehaviour->IsEnabled())
		{
			scriptBehaviour->OnEnable();
			scriptBehaviour->Start();
			m_ScriptBehaviours.push_back(scriptBehaviour);

			it = m_RegisterQueue.erase(it);
		}

		else
		{
			++it;
		}
	}
}

void engine::ScriptBehaviourManager::FlushDestroyScriptBehaviours()
{
	for (auto it = m_ScriptBehaviours.begin(); it != m_ScriptBehaviours.end();)
	{
		const auto scriptBehaviour = *it;

		if (scriptBehaviour->IsDestroyed())
		{
			scriptBehaviour->OnDestroy();

			if (const auto owner = (*it)->GetGameObject().lock())
			{
				owner->RemoveComponent(*it);
			}

			it = m_ScriptBehaviours.erase(it);
		}

		else
		{
			++it;
		}
	}
}

void engine::ScriptBehaviourManager::Release()
{
	m_ScriptBehaviours.clear();
	m_RegisterQueue.clear();
}
