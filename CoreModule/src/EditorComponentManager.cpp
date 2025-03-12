#include "EditorComponentManager.h"

#include "GameObject.h"
#include "Renderer.h"
#include "UI.h"

engine::editor::EditorComponentManager::EditorComponentManager()
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

	for (const auto& ui : m_UIs)
	{
		if (auto owner = ui->GetGameObject().lock())
		{
			if (owner->IsActive())
			{
				ui->RenderUI(context);
			}
		}
	}
}
