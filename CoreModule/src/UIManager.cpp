#include "UIManager.h"

#include "D3D11Manager.h"
#include "Material.h"
#include "UI.h"

engine::UIManager::UIManager() : m_MaxSort(INT_MIN), m_MinSort(INT_MAX), m_bDirty(true)
{
}

engine::UIManager::~UIManager()
{

}

engine::SharedPtr<DirectX::SpriteFont> engine::UIManager::GetFont(const _wstring& name)
{
	auto it = m_Fonts.find(name);

	if (it != m_Fonts.end())
	{
		return it->second;
	}

	return nullptr;
}

void engine::UIManager::Initialize()
{
	AddFont(L"HUGoth150", L"..\\Client\\Assets\\Resource\\Font\\HUGoth150.spritefont");
	m_Batch = std::make_shared<DirectX::SpriteBatch>(D3D11Manager::GetInstance().GetContext().Get());
}

void engine::UIManager::UpdateUI()
{
	m_MaxSort = INT_MIN;
	m_MinSort = INT_MAX;

	if (!m_UIMap.empty())
	{
		m_MaxSort = std::max(m_MaxSort, m_UIMap.rbegin()->first);
	}

	if (!m_TextUIMap.empty())
	{
		m_MaxSort = std::max(m_MaxSort, m_TextUIMap.rbegin()->first);
	}

	if (!m_UIMap.empty())
	{
		m_MinSort = std::min(m_MinSort, m_UIMap.begin()->first);
	}

	if (!m_TextUIMap.empty())
	{
		m_MinSort = std::min(m_MinSort, m_TextUIMap.begin()->first);
	}
}

void engine::UIManager::Render(const ComPtr<ID3D11DeviceContext>& context)
{
	UpdateUI();

	ComPtr<ID3D11BlendState>   blendState;
	FLOAT                      blendFactor[4] = {};
	UINT                       sampleMask = 0;

	ComPtr<ID3D11DepthStencilState> backupDepthState;
	UINT stencilRef = 0;

	context->OMGetBlendState(blendState.GetAddressOf(), blendFactor, &sampleMask);
	context->OMGetDepthStencilState(backupDepthState.GetAddressOf(), &stencilRef);

	D3D11Manager::GetInstance().SetUIAlphaBlendMode();

	// TODO : 직교 투영을 위한 View, Proj 행렬을 constant buffer에 던져줘야함.

	const _float winSizeX = static_cast<_float>(D3D11Manager::GetInstance().GetWinSizeX());
	const _float winSizeY = static_cast<_float>(D3D11Manager::GetInstance().GetWinSizeY());

	_float4X4 viewMat, projMat;
	XMStoreFloat4x4(&viewMat, DirectX::XMMatrixIdentity());
	XMStoreFloat4x4(&projMat,DirectX::XMMatrixOrthographicLH(winSizeX, winSizeY, -1.f, 1.f));

	for (_int sort = m_MinSort; sort <= m_MaxSort; ++sort)
	{
		auto uiIter = m_UIMap.find(sort);

		if (uiIter != m_UIMap.end())
		{
			for (const auto& ui : uiIter->second)
			{
				if (ui->IsEnabled())
				{
					auto material = ui->GetMaterial();

					if (material)
					{
						material->SetMatrix("g_ViewMatrix", viewMat);
						material->SetMatrix("g_ProjMatrix", projMat);
					}

					ui->RenderUI(context);
				}
			}
		}

		auto textIter = m_TextUIMap.find(sort);

		if (textIter != m_TextUIMap.end() && !textIter->second.empty())
		{
			ComPtr<ID3D11BlendState>   prevBlendState;
			FLOAT                      prevBlendFactor[4] = {};
			UINT                       prevSampleMask = 0;

			ComPtr<ID3D11DepthStencilState> prevDepthState;
			UINT                       prevStencilRef = 0;

			context->OMGetBlendState(prevBlendState.GetAddressOf(), prevBlendFactor, &prevSampleMask);
			context->OMGetDepthStencilState(prevDepthState.GetAddressOf(), &prevStencilRef);

			m_Batch->Begin();

			for (const auto& text : textIter->second)
			{
				if (text->IsEnabled())
				{
					text->RenderUI(context);
				}
			}

			m_Batch->End();

			context->OMSetBlendState(prevBlendState.Get(), prevBlendFactor, prevSampleMask);
			context->OMSetDepthStencilState(prevDepthState.Get(), prevStencilRef);
		}
	}

	context->OMSetBlendState(blendState.Get(), blendFactor, sampleMask);
	context->OMSetDepthStencilState(backupDepthState.Get(), stencilRef);
}

void engine::UIManager::AddUI(const SharedPtr<UI>& ui, const _bool isText)
{
	if (isText)
	{
		m_RegisterTextQueue.push_back(ui);
	}

	else
	{
		m_RegisterQueue.push_back(ui);
	}
}

void engine::UIManager::AddFont(const _wstring& name, const _wstring& path)
{
	SharedPtr<DirectX::SpriteFont> font = std::make_shared<DirectX::SpriteFont>(D3D11Manager::GetInstance().GetDevice().Get(), path.c_str());

	m_Fonts.emplace(name, font);
}

void engine::UIManager::OnSortingChanged(const SharedPtr<UI>& ui, const _int oldSort, const _int newSort, const _bool isText)
{
	if (!isText)
	{
		auto& oldVec = m_UIMap[oldSort];
		oldVec.erase(std::remove(oldVec.begin(), oldVec.end(), ui), oldVec.end());
		if (oldVec.empty())
		{
			m_UIMap.erase(oldSort);
		}

		m_UIMap[newSort].push_back(ui);
	}

	else
	{
		auto& oldVec = m_TextUIMap[oldSort];
		oldVec.erase(std::remove(oldVec.begin(), oldVec.end(), ui), oldVec.end());
		if (oldVec.empty())
		{
			m_TextUIMap.erase(oldSort);
		}

		m_TextUIMap[newSort].push_back(ui);
	}
}

void engine::UIManager::RegisterUI()
{
	for (auto it = m_RegisterQueue.begin(); it != m_RegisterQueue.end();)
	{
		SharedPtr<UI> ui = *it;

		if (ui->IsEnabled())
		{
			_int sort = ui->GetSorting();

			m_UIMap[sort].push_back(ui);

			it = m_RegisterQueue.erase(it);

			SetDirty(true);
		}

		else
		{
			++it;
		}
	}

	for (auto it = m_RegisterTextQueue.begin(); it != m_RegisterTextQueue.end();)
	{
		SharedPtr<UI> ui = *it;

		if (ui->IsEnabled())
		{
			_int sort = ui->GetSorting();

			m_TextUIMap[sort].push_back(ui);

			it = m_RegisterTextQueue.erase(it);

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
	for (auto& pair : m_UIMap)
	{
		for (auto it = pair.second.begin(); it!= pair.second.end();)
		{
			const auto ui = *it;

			if (ui->IsDestroyed())
			{
				if (const auto owner = (*it)->GetGameObject().lock())
				{
					owner->RemoveComponent(*it);
				}

				it = pair.second.erase(it);

				SetDirty(true);
			}

			else
			{
				++it;
			}
		}
	}

	for (auto& pair : m_TextUIMap)
	{
		for (auto it = pair.second.begin(); it != pair.second.end();)
		{
			const auto ui = *it;

			if (ui->IsDestroyed())
			{
				if (const auto owner = (*it)->GetGameObject().lock())
				{
					owner->RemoveComponent(*it);
				}

				it = pair.second.erase(it);

				SetDirty(true);
			}

			else
			{
				++it;
			}
		}
	}

}

void engine::UIManager::Release()
{
	m_UIMap.clear();
	m_TextUIMap.clear();

	m_RegisterQueue.clear();
}

IMPLEMENT_SINGLETON(engine::UIManager)
