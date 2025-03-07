#include "Component.h"

#include "GameObject.h"

engine::SharedPtr<engine::Transform> engine::Component::GetTransform() const
{
	if (auto gameObject = m_Owner.lock())
	{
		return gameObject->GetTransform();
	}

	return nullptr;
}
