#include "Behaviour.h"

bool engine::Behaviour::IsEnabled() const
{
	const auto owner = m_Owner.lock();

	if (!owner || !owner->IsActive())
	{
		return false;
	}

	return m_bEnabled;
}
