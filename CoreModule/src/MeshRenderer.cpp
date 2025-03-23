#include "MeshRenderer.h"

DEFINE_REGISTER_COMPONENT(MeshRenderer)

engine::MeshRenderer::MeshRenderer(const SharedPtr<GameObject>& owner, const _string& name)
	: Renderer(owner, name)
{

}

engine::MeshRenderer::~MeshRenderer()
{
}

engine::MeshRenderer::MeshRenderer(const MeshRenderer& rhs)
	: Renderer(rhs)
{

}

void engine::MeshRenderer::Bind(const ComPtr<ID3D11DeviceContext>& context)
{
	
}

void engine::MeshRenderer::Render(const ComPtr<ID3D11DeviceContext>& context)
{

}

void engine::MeshRenderer::to_json(nlohmann::ordered_json& j)
{
}

void engine::MeshRenderer::from_json(const nlohmann::ordered_json& j)
{
}

void engine::MeshRenderer::Destroy()
{
	m_bDestroyed = true;
}
