#include "UIManager.h"

#include "D3D11Manager.h"
#include "Material.h"
#include "UI.h"

engine::UIManager::UIManager() : m_bDirty(true)
{
}

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

	const _float winSizeX = static_cast<_float>(D3D11Manager::GetInstance().GetWinSizeX());
	const _float winSizeY = static_cast<_float>(D3D11Manager::GetInstance().GetWinSizeY());

	_float4X4 viewMat, projMat;
	XMStoreFloat4x4(&viewMat, DirectX::XMMatrixIdentity());
	XMStoreFloat4x4(&projMat, DirectX::XMMatrixOrthographicLH(winSizeX, winSizeY, -1.f, 1.f));

	// TODO : Render Option에 맞게 렌더링 순서 조절 (Alpha Blend)
	// TODO : Sorting Num에 맞게 UI들 Sorting 후 렌더링

	// Dirty Mask로 UI의 Sorting Order가 변경되거나 UI가 생성, 삭제 됐을 때만
	// Sorting을 진행하여 필요할 때만 Sorting 수행.

	for (const auto& ui : m_UIs)
	{
		if (ui->IsEnabled())
		{
			auto material = ui->GetMaterial();
			material->SetMatrix("g_ViewMat", viewMat);
			material->SetMatrix("g_ProjMat", projMat);

			ui->RenderUI(context);
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

			SetDirty(true);
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

			SetDirty(true);
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
