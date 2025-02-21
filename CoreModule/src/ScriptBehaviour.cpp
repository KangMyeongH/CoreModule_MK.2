#include "ScriptBehaviour.h"

void engine::ScriptBehaviour::SetEnable(const bool enabled)
{
	if (m_bEnabled != enabled)
	{
		if (enabled)
		{
			OnEnable();
		}

		else
		{
			OnDisable();
		}

		m_bEnabled = enabled;
	}
}

void engine::ScriptBehaviour::Destroy()
{
	if (!m_bDestroyed)
	{
		m_bDestroyed = true;
	}
}

void engine::ScriptBehaviour::registerComponent()
{
	ScriptBehaviourManager::GetInstance().AddScriptBehaviour(std::dynamic_pointer_cast<ScriptBehaviour>(shared_from_this()));
}
