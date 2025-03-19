#include "UI.h"

#include "UIManager.h"

engine::UI::UI(const SharedPtr<GameObject>& owner) : Behaviour(owner), m_SortingOrder(0)
{

}

engine::UI::UI(const UI& rhs)
	: Behaviour(rhs),
	m_SortingOrder(rhs.m_SortingOrder)
{

}

engine::_uint engine::UI::GetSorting() const
{
	return m_SortingOrder;
}

void engine::UI::SetSorting(const _uint sort)
{
	if (m_SortingOrder != sort)
	{
		m_SortingOrder = sort;

		UIManager::GetInstance().SetDirty(true);
	}
}

void engine::UI::Destroy()
{
	m_bDestroyed = true;
}

void engine::UI::registerComponent(ApplicationMode mode)
{
	if (mode == CLIENT)
	{
		UIManager::GetInstance().AddUI(std::static_pointer_cast<UI>(shared_from_this()));
	}
}
