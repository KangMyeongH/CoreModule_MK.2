#include "UIManager.h"

#include "UI.h"

engine::UIManager::~UIManager()
{
	Release();
}

void engine::UIManager::UpdateUI()
{

}

void engine::UIManager::Render(const ComPtr<ID3D11DeviceContext>& context)
{
	// TODO : 직교 투영을 위한 View, Proj 행렬을 constant buffer에 던져줘야함.

	// TODO : Render Option에 맞게 렌더링 순서 조절 (Alpha Blend)

	for (const auto& ui : m_UIs)
	{
		if (ui->IsEnabled())
		{
			ui->RenderUI();
		}
	}
}

void engine::UIManager::AddUI(const SharedPtr<UI>& ui)
{
	m_RegisterQueue.push_back(ui);
}

void engine::UIManager::RegisterUI()
{
	for (auto it = m_RegisterQueue.begin(); it != m_RegisterQueue.end();)
	{
		SharedPtr<UI> ui = *it;

		if (ui->IsEnabled())
		{
			m_UIs.push_back(ui);

			it = m_RegisterQueue.erase(it);
		}

		else
		{
			++it;
		}
	}
}

void engine::UIManager::FlushDestroyUI()
{
	for (auto it = m_UIs.begin(); it != m_UIs.end();)
	{
		const auto ui = *it;

		if (ui->IsDestroyed())
		{
			if (const auto owner = (*it)->GetGameObject().lock())
			{
				owner->RemoveComponent(*it);
			}

			it = m_UIs.erase(it);
		}

		else
		{
			++it;
		}
	}
}

void engine::UIManager::Release()
{
	m_UIs.clear();
	m_RegisterQueue.clear();
}

IMPLEMENT_SINGLETON(engine::UIManager)
