#include "RenderManager.h"

#include "Camera.h"
#include "GameObject.h"
#include "Renderer.h"

IMPLEMENT_SINGLETON(engine::RenderManager)

engine::RenderManager::RenderManager()
{
}

engine::RenderManager::~RenderManager()
{
}

void engine::RenderManager::AddCamera(const SharedPtr<Camera>& camera)
{
	m_Cameras.push_back(camera);
}

void engine::RenderManager::AddRenderer(const SharedPtr<Renderer>& renderer)
{
	m_RegisterQueue.push_back(renderer);
}

void engine::RenderManager::UpdateMainCamera()
{
	if (auto mainCam = m_MainCamera.lock())
	{
		mainCam->UpdateCamera(m_ViewMat, m_ProjMat);
	}

	else
	{
		// TODO : MainCam이 없을 때 예외 처리.
	}
}

void engine::RenderManager::Render(const ComPtr<ID3D11DeviceContext>& context)
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
}

void engine::RenderManager::RegisterRenderer()
{
	for (auto it = m_RegisterQueue.begin(); it != m_RegisterQueue.end();)
	{
		SharedPtr<Renderer> renderer = *it;
		m_Renderers.push_back(renderer);
	}
}

void engine::RenderManager::FlushDestroyRenderer()
{
	for (auto it = m_Renderers.begin(); it != m_Renderers.end();)
	{
		SharedPtr<Renderer> renderer = *it;
		if (renderer->IsDestroyed())
		{
			if (const auto owner = renderer->GetGameObject().lock())
			{
				owner->RemoveComponent(renderer);
			}

			it = m_Renderers.erase(it);
		}

		else
		{
			++it;
		}
	}
}

void engine::RenderManager::FlushDestroyCamera()
{
	for (auto it = m_Cameras.begin(); it != m_Cameras.end();)
	{
		const auto camera = *it;

		if (camera->IsDestroyed())
		{
			if (const auto owner = camera->GetGameObject().lock())
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
}

void engine::RenderManager::Release()
{
	m_Renderers.clear();
	m_RegisterQueue.clear();

	m_MainCamera.reset();
	m_Cameras.clear();
}
