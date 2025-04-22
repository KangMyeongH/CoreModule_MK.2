#include "TextUI.h"

#include "EditorComponentManager.h"
#include "UIManager.h"

DEFINE_REGISTER_COMPONENT(TextUI)

engine::TextUI::TextUI(const SharedPtr<GameObject>& owner)
	: UI(owner, "TextUI"), m_Color({1.f, 1.f, 1.f, 1.f}), m_Size(1.f)
{
}

engine::TextUI::TextUI(const TextUI& rhs)
	: UI(rhs), m_Font(rhs.m_Font), m_Batch(rhs.m_Batch), m_FontName(rhs.m_FontName), m_Text(rhs.m_Text), m_Color(rhs.m_Color), m_Size(rhs.m_Size)
{

}

void engine::TextUI::SetFont(const _wstring& font)
{
	m_Font = UIManager::GetInstance().GetFont(font);
	m_FontName = font;
}

void engine::TextUI::SetSorting(_int sort, ApplicationMode mode)
{
	if (m_SortingOrder == sort)
	{
		return;
	}

	_int oldSort = m_SortingOrder;
	m_SortingOrder = sort;

	if (mode == CLIENT)
	{
		UIManager::GetInstance().OnSortingChanged(std::static_pointer_cast<UI>(shared_from_this()), oldSort, m_SortingOrder, true);
	}

	else if (mode == EDITOR)
	{
		editor::EditorComponentManager::GetInstance().OnSortingChanged(std::static_pointer_cast<UI>(shared_from_this()), oldSort, m_SortingOrder, true);
	}
}

engine::_bool engine::TextUI::IsMouseHovered()
{
	return false;
}

engine::_bool engine::TextUI::IsButtonDown()
{
	return false;
}

engine::_bool engine::TextUI::IsButtonHold()
{
	return false;
}

engine::_bool engine::TextUI::IsButtonUp()
{
	return false;
}

void engine::TextUI::Update()
{

}

HRESULT engine::TextUI::InputAssembler(const ComPtr<ID3D11DeviceContext>& context)
{
	return S_OK;
}

void engine::TextUI::RenderUI(const ComPtr<ID3D11DeviceContext>& context)
{
	auto transform = GetTransform();

	_float2 	position = { transform->Position().Value.x, transform->Position().Value.y };
	_vector 	color = XMLoadFloat4(&m_Color);
	_float	 	rotation = transform->Rotation().Value.z;
	_float2 	origin = { 0,0 };

	if (m_Font)
	{
		m_Font->DrawString(m_Batch.get(), m_Text.c_str(), position, color, rotation, origin, m_Size);
	}
}

void engine::TextUI::registerComponent(ApplicationMode mode)
{
	if (mode == CLIENT)
	{
		UIManager::GetInstance().AddUI(std::static_pointer_cast<UI>(shared_from_this()), true);
	}

	if (mode == EDITOR)
	{
		editor::EditorComponentManager::GetInstance().AddComponent(std::static_pointer_cast<UI>(shared_from_this()));
	}

	m_Batch = UIManager::GetInstance().GetBatch();
}


void engine::TextUI::to_json(nlohmann::ordered_json& j)
{
	std::string type = "TextUI";
	j = nlohmann::ordered_json{
		{"type", type},
		{"enable", m_bEnabled},
		{"sortingOrder", m_SortingOrder},
		{"font", m_FontName},
		{"text", m_Text},
		{"size", m_Size}
	};
}

void engine::TextUI::from_json(const nlohmann::ordered_json& j)
{
	if (j.contains("enable"))
	{
		j.at("enable").get_to(m_bEnabled);
	}

	if (j.contains("sortingOrder"))
	{
		j.at("sortingOrder").get_to(m_SortingOrder);
	}

	if (j.contains("font"))
	{
		j.at("font").get_to(m_FontName);
		SetFont(m_FontName);
	}

	if (j.contains("text"))
	{
		j.at("text").get_to(m_Text);
	}

	if (j.contains("size"))
	{
		j.at("size").get_to(m_Size);
	}
}
