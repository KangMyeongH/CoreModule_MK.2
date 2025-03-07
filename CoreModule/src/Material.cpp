#include "Material.h"

#include "D3D11Manager.h"

engine::Material::Material(const SharedPtr<Renderer>& owner)
	: Object("Material"),
	  m_Owner(owner),
	  m_DiffuseTexture(nullptr), m_DiffuseColor(), m_AmbientColor()
{
}

engine::Material::~Material()
{
	m_Owner.reset();
	m_DiffuseTexture->Release();
}

engine::Material::Material(const Material& rhs)
	: Object(rhs),
	  m_DiffuseTexture(nullptr), m_DiffuseColor(), m_AmbientColor()
{
}

void engine::Material::SetDiffuseTexture(const _wstring& path)
{
	SafeRelease(m_DiffuseTexture);

	if (FAILED(D3D11Manager::GetInstance().CreateTexture(path, &m_DiffuseTexture)))
	{
		m_DiffuseTexture = nullptr;
		return;
	}

	m_DiffuseTexture->AddRef();
}

engine::SharedPtr<engine::Material> engine::Material::Create(const SharedPtr<Renderer>& renderer)
{
	return {
		new Material(renderer),
		[](const Material* ptr) { delete ptr; }
	};
}

void engine::Material::Destroy()
{
	if (auto owner = m_Owner.lock())
	{
		
	}
}
