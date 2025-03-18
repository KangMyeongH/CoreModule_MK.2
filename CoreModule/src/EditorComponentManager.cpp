#include "EditorComponentManager.h"

#include "Camera.h"
#include "D3D11Manager.h"
#include "GameObject.h"
#include "Material.h"
#include "Renderer.h"
#include "UI.h"

engine::editor::EditorComponentManager::EditorComponentManager() : m_DirtyFlag(true)
{
}

engine::editor::EditorComponentManager::~EditorComponentManager()
{
}

IMPLEMENT_SINGLETON(engine::editor::EditorComponentManager)

void engine::editor::EditorComponentManager::Render(const ComPtr<ID3D11DeviceContext>& context)
{
	for (const auto& renderer : m_Renderers)
	{
		if (auto owner = renderer->GetGameObject().lock())
		{
			if (owner->IsActive())
			{
				renderer->Render(context);
			}
		}
	}

	RenderUIComponent(context);
}

void engine::editor::EditorComponentManager::RenderUIComponent(const ComPtr<ID3D11DeviceContext>& context)
{
	const _float winSizeX = static_cast<_float>(D3D11Manager::GetInstance().GetWinSizeX());
	const _float winSizeY = static_cast<_float>(D3D11Manager::GetInstance().GetWinSizeY());

	_float4X4 viewMat, projMat;
	XMStoreFloat4x4(&viewMat, DirectX::XMMatrixIdentity());
	XMStoreFloat4x4(&projMat, DirectX::XMMatrixOrthographicLH(winSizeX, winSizeY, -1.f, 1.f));

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

void engine::editor::EditorComponentManager::AddComponent(const SharedPtr<GameObject>& owner, const SharedPtr<Component>& component)
{
	if (auto ui = std::dynamic_pointer_cast<UI>(component))
	{
		m_UIs.push_back(ui);
	}

	else if (auto renderer = std::dynamic_pointer_cast<Renderer>(component))
	{
		m_Renderers.push_back(renderer);
	}

	else if (auto camera = std::dynamic_pointer_cast<Camera>(component))
	{
		m_Cameras.push_back(camera);
	}

	else
	{
		m_Components.push_back(component);
	}

	component->SetOwner(owner);
	auto& components = owner->GetComponents();
	components[typeid(*component)].push_back(component);
	component->registerComponent(EDITOR);
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
