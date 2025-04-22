#include "UI.h"

#include "EditorComponentManager.h"
#include "UIManager.h"

engine::UI::UI(const SharedPtr<GameObject>& owner, const _string& name) : Behaviour(owner), m_SortingOrder(0)
{

}

engine::UI::UI(const UI& rhs)
	: Behaviour(rhs),
	m_SortingOrder(rhs.m_SortingOrder)
{

}

engine::_int engine::UI::GetSorting() const
{
	return m_SortingOrder;
}

void engine::UI::SetSorting(const _int sort, ApplicationMode mode)
{
	if (m_SortingOrder == sort)
	{
		return;
	}

	_int oldSort = m_SortingOrder;
	m_SortingOrder = sort;

	if (mode == CLIENT)
	{
		UIManager::GetInstance().OnSortingChanged(std::static_pointer_cast<UI>(shared_from_this()), oldSort, m_SortingOrder, false);
	}

	else if (mode == EDITOR)
	{
		editor::EditorComponentManager::GetInstance().OnSortingChanged(std::static_pointer_cast<UI>(shared_from_this()), oldSort, m_SortingOrder, false);
	}
}

void engine::UI::Destroy()
{
	m_bDestroyed = true;
}

void engine::UI::registerComponent(ApplicationMode mode)
{

}
