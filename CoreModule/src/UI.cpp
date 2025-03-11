#include "UI.h"

#include "UIManager.h"

engine::UI::UI(const SharedPtr<GameObject>& owner) : Behaviour(owner)
{

}

engine::UI::UI(const UI& rhs) : Behaviour(rhs)
{

}

void engine::UI::registerComponent()
{
	UIManager::GetInstance().AddUI(std::static_pointer_cast<UI>(shared_from_this()));
}
