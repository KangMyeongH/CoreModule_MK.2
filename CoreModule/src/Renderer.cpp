#include "Renderer.h"

#include "Material.h"
#include "RenderManager.h"

engine::Renderer::Renderer(const SharedPtr<GameObject>& owner, const _string& name) : Behaviour(owner, name)
{

}

engine::Renderer::Renderer(const Renderer& rhs) : Behaviour(rhs)
{

}

void engine::Renderer::registerComponent(ApplicationMode mode)
{
	if (mode == CLIENT)
	{
		RenderManager::GetInstance().AddRenderer(std::static_pointer_cast<Renderer>(shared_from_this()));
	}
}