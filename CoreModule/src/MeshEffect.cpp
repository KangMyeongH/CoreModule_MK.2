#include "MeshEffect.h"

#include "Material.h"
#include "Mesh.h"

DEFINE_REGISTER_COMPONENT(MeshEffect)

engine::MeshEffect::MeshEffect(const SharedPtr<GameObject>& owner, const _string& name)
	: Effect(owner, name)
{

}

engine::MeshEffect::MeshEffect(const MeshEffect& rhs)
	: Effect(rhs)
{
}

void engine::MeshEffect::InputAssembler(ID3D11DeviceContext* context)
{
	if (m_Mesh)
	{
		m_Mesh->InputAssembler(context);
	}
}

void engine::MeshEffect::Bind(ID3D11DeviceContext* context)
{
	if (m_Material)
	{
		if (m_Material->GetShader())
		{
			m_Material->SetMatrix("g_WorldMatrix", GetTransform()->GetWorldMatrix());
			m_Material->Bind(context);
		}
	}
}

void engine::MeshEffect::Render(ID3D11DeviceContext* context)
{
	if (m_Mesh)
	{
		m_Mesh->Render(context);
	}
}

void engine::MeshEffect::Destroy()
{
	m_bDestroyed = true;
}

engine::SharedPtr<engine::Component> engine::MeshEffect::Clone() const
{
	SharedPtr<MeshEffect> clone(CLONE_SHARED_PTR(MeshEffect));

	clone->m_Material = m_Material->Clone(clone);

	return clone;
}

void engine::MeshEffect::registerComponent(ApplicationMode mode)
{
	Effect::registerComponent(mode);
}
