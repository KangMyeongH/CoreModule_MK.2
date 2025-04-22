#include "EditorComponentManager.h"

#include "Camera.h"
#include "Collider.h"
#include "D3D11Manager.h"
#include "DebugRenderManager.h"
#include "EditorCore.h"
#include "GameObject.h"
#include "Material.h"
#include "Renderer.h"
#include "TextUI.h"
#include "UI.h"
#include "UIManager.h"

engine::editor::EditorComponentManager::EditorComponentManager() : m_ViewMat(), m_ProjMat(), m_DirtyFlag(true)
{
}

engine::editor::EditorComponentManager::~EditorComponentManager()
= default;

IMPLEMENT_SINGLETON(engine::editor::EditorComponentManager)

void engine::editor::EditorComponentManager::Initialize()
{
	AddFont(L"HUGoth150", L"..\\Client\\Assets\\Resource\\Font\\HUGoth150.spritefont");
	m_Batch = UIManager::GetInstance().GetBatch();
}

void engine::editor::EditorComponentManager::Render(const ComPtr<ID3D11DeviceContext>& context, const _float4X4& viewMat, const _float4X4& projMat)
{
	Vector3 camPos = EditorCore::GetInstance().GetEditorCamera().GetCameraPos();
	_float4 finalCamPos = { camPos.Value.x, camPos.Value.y, camPos.Value.z, 1.f };
	for (const auto& renderer : m_Renderers)
	{
		if (auto owner = renderer->GetGameObject().lock())
		{
			if (owner->IsActive())
			{
				auto materials = renderer->GetMaterials();

				for (auto material : materials)
				{
					if (material.second->GetShader())
					{
						material.second->SetMatrix("g_ViewMatrix", viewMat);
						material.second->SetMatrix("g_ProjMatrix", projMat);
						material.second->SetFloat4("CameraPosition", finalCamPos);
					}
				}

				renderer->Render(context);
			}
		}
	}

	RenderCollider(context, viewMat, projMat);

	RenderUIComponent(context);
}

void engine::editor::EditorComponentManager::RenderUIComponent(const ComPtr<ID3D11DeviceContext>& context)
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

	ComPtr<ID3D11BlendState>   blendState;
	FLOAT                      blendFactor[4] = {};
	UINT                       sampleMask = 0;

	ComPtr<ID3D11DepthStencilState> backupDepthState;
	UINT stencilRef = 0;

	context->OMGetBlendState(blendState.GetAddressOf(), blendFactor, &sampleMask);
	context->OMGetDepthStencilState(backupDepthState.GetAddressOf(), &stencilRef);

	D3D11Manager::GetInstance().SetUIAlphaBlendMode();

	const _float winSizeX = static_cast<_float>(D3D11Manager::GetInstance().GetWinSizeX());
	const _float winSizeY = static_cast<_float>(D3D11Manager::GetInstance().GetWinSizeY());

	_float4X4 viewMat, projMat;
	XMStoreFloat4x4(&viewMat, DirectX::XMMatrixIdentity());
	XMStoreFloat4x4(&projMat, DirectX::XMMatrixOrthographicLH(winSizeX, winSizeY, -1.f, 1.f));

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

void engine::editor::EditorComponentManager::RenderCollider(const ComPtr<ID3D11DeviceContext>& context,
	const _float4X4& viewMat, const _float4X4& projMat) const
{
	DebugRenderManager::GetInstance().RenderCollider(m_Colliders, context, viewMat, projMat);
}

void engine::editor::EditorComponentManager::AddComponent(const SharedPtr<GameObject>& owner, const SharedPtr<Component>& component)
{
	if (auto ui = std::dynamic_pointer_cast<UI>(component))
	{
		_int sort = ui->GetSorting();

		if (auto text = std::dynamic_pointer_cast<TextUI>(ui))
		{
			m_TextUIMap[sort].push_back(text);
		}

		else
		{
			m_UIMap[sort].push_back(ui);
		}
	}

	else if (auto renderer = std::dynamic_pointer_cast<Renderer>(component))
	{
		m_Renderers.push_back(renderer);
	}

	else if (auto camera = std::dynamic_pointer_cast<Camera>(component))
	{
		if (!m_MainCamera.lock())
		{
			m_MainCamera = camera;
		}

		m_Cameras.push_back(camera);
	}

	else if (auto collider = std::dynamic_pointer_cast<Collider>(component))
	{
		m_Colliders.push_back(collider);
	}

	else
	{
		m_Components.push_back(component);
	}

	component->SetOwner(owner);
	auto& components = owner->GetComponents();
	if (components.find(typeid(*component)) == components.end())
	{
		components[typeid(*component)].push_back(component);
	}
	component->registerComponent(EDITOR);
}

void engine::editor::EditorComponentManager::AddComponent(const SharedPtr<Component>& component)
{
	if (auto ui = std::dynamic_pointer_cast<UI>(component))
	{
		_int sort = ui->GetSorting();

		if (auto text = std::dynamic_pointer_cast<TextUI>(ui))
		{
			m_TextUIMap[sort].push_back(text);
		}

		else
		{
			m_UIMap[sort].push_back(ui);
		}
	}

	else if (auto renderer = std::dynamic_pointer_cast<Renderer>(component))
	{
		m_Renderers.push_back(renderer);
	}

	else if (auto camera = std::dynamic_pointer_cast<Camera>(component))
	{
		if (!m_MainCamera.lock())
		{
			m_MainCamera = camera;
		}

		m_Cameras.push_back(camera);
	}

	else if (auto collider = std::dynamic_pointer_cast<Collider>(component))
	{
		m_Colliders.push_back(collider);
	}

	else
	{
		m_Components.push_back(component);
	}
}

void engine::editor::EditorComponentManager::AddFont(const _wstring& name, const _wstring& path)
{
	SharedPtr<DirectX::SpriteFont> font = std::make_shared<DirectX::SpriteFont>(D3D11Manager::GetInstance().GetDevice().Get(), path.c_str());

	m_Fonts.emplace(name, font);
}

void engine::editor::EditorComponentManager::OnSortingChanged(const SharedPtr<UI>& ui, const _int oldSort, const _int newSort,
                                                              const _bool isText)
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


void engine::editor::EditorComponentManager::FlushDestroyComponent()
{
	for (auto it = m_Renderers.begin(); it != m_Renderers.end();)
	{
		const auto renderer = *it;

		if (renderer->IsDestroyed())
		{
			if (const auto owner = (*it)->GetGameObject().lock())
			{
				owner->RemoveComponent(*it);
			}

			it = m_Renderers.erase(it);
		}

		else
		{
			++it;
		}
	}

	for (auto& pair : m_UIMap)
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

	for (auto it = m_Cameras.begin(); it != m_Cameras.end();)
	{
		const auto camera = *it;

		if (camera->IsDestroyed())
		{
			if (const auto owner = (*it)->GetGameObject().lock())
			{
				owner->RemoveComponent(camera);
			}

			it = m_Cameras.erase(it);
		}

		else
		{
			++it;
		}
	}

	for (auto it = m_Components.begin(); it != m_Components.end();)
	{
		const auto component = *it;

		if (component->IsDestroyed())
		{
			if (const auto owner = (*it)->GetGameObject().lock())
			{
				owner->RemoveComponent(component);
			}

			it = m_Components.erase(it);
		}

		else
		{
			++it;
		}
	}
}

void engine::editor::EditorComponentManager::Release()
{
	m_UIMap.clear();
	m_TextUIMap.clear();
	m_Renderers.clear();
	m_Cameras.clear();
	m_Components.clear();
}
