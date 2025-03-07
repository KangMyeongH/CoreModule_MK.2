#include "RenderManager.h"

#include "Camera.h"
#include "GameObject.h"

IMPLEMENT_SINGLETON(engine::RenderManager)

engine::RenderManager::RenderManager(): m_VSConstantBuffer()
{
}

engine::RenderManager::~RenderManager()
{
	Release();
}

void engine::RenderManager::AddCamera(const SharedPtr<Camera>& camera)
{
	m_Cameras.push_back(camera);
}

void engine::RenderManager::UpdateMainCamera()
{
	if (auto mainCam = m_MainCamera.lock())
	{
		mainCam->UpdateCamera(&m_VSConstantBuffer);
	}
}

void engine::RenderManager::Render()
{

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
	m_MainCamera.reset();
	m_Cameras.clear();
}
