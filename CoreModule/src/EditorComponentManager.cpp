#include "EditorComponentManager.h"

#include "Camera.h"
#include "Collider.h"
#include "D3D11Manager.h"
#include "DebugRenderManager.h"
#include "EditorCore.h"
#include "GameObject.h"
#include "Material.h"
#include "Renderer.h"
#include "UI.h"

engine::editor::EditorComponentManager::EditorComponentManager() : m_ViewMat(), m_ProjMat(), m_DirtyFlag(true)
{
}

engine::editor::EditorComponentManager::~EditorComponentManager()
= default;

IMPLEMENT_SINGLETON(engine::editor::EditorComponentManager)

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
	const _float winSizeX = static_cast<_float>(D3D11Manager::GetInstance().GetWinSizeX());
	const _float winSizeY = static_cast<_float>(D3D11Manager::GetInstance().GetWinSizeY());

	_float4X4 viewMat, projMat;
	XMStoreFloat4x4(&viewMat, DirectX::XMMatrixIdentity());
	XMStoreFloat4x4(&projMat,DirectX::XMMatrixOrthographicLH(winSizeX, winSizeY, -1.f, 1.f));

	for (const auto& ui : m_UIs)
	{
		if (ui->IsEnabled())
		{
			auto material = ui->GetMaterial();
			material->SetMatrix("g_ViewMatrix", viewMat);
			material->SetMatrix("g_ProjMatrix", projMat);

			ui->RenderUI(context);
		}
	}
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
		m_UIs.push_back(ui);
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
		m_UIs.push_back(ui);
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

void engine::editor::EditorComponentManager::Release()
{
	m_UIs.clear();
	m_Renderers.clear();
	m_Cameras.clear();
	m_Components.clear();
}
