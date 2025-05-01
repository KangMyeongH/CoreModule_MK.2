#include "Light.h"

#include "EditorComponentManager.h"
#include "Material.h"
#include "RenderManager.h"

DEFINE_REGISTER_COMPONENT(Light)

engine::Light::Light(const SharedPtr<GameObject>& owner, const _string& name)
	: Behaviour(owner, name), m_Type(LightType_Point), m_Color({1.f, 1.f, 1.f, 1.f}), m_Intensity(1.f), m_Range(10.f), m_SpotAngle(30.f)
{
}

engine::Light::~Light() = default;

engine::Light::Light(const Light& rhs)
	: Behaviour(rhs), m_Type(rhs.m_Type), m_Color(rhs.m_Color), m_Intensity(rhs.m_Intensity), m_Range(rhs.m_Range), m_SpotAngle(rhs.m_SpotAngle)
{

}

engine::LightDesc engine::Light::GetLightDesc() const
{
	_float3 pos = GetTransform()->Position().Value;
	_float3 vDir = GetTransform()->Forward().Value;
	_float4 dir = _float4{ vDir.x, vDir.y, vDir.z, 0.f };


	LightDesc desc{};
	desc.Type = m_Type;
	desc.Position = _float4(pos.x, pos.y, pos.z, 1.f);
	desc.Dir = dir;
	desc.Color = m_Color;
	desc.Intensity = m_Intensity;
	desc.Range = m_Range;
	desc.SpotAngle = m_SpotAngle;

	return desc;
}

void engine::Light::BindLight(const SharedPtr<Material>& material)
{
	// TODO : 임시 바인딩임. 디퍼드 쉐이더 구현하면 수정해야함.

	auto desc = GetLightDesc();

	switch (m_Type)
	{
	case LightType_Spot:
		break;
	case LightType_Directional:
		material->SetFloat4("DirLight_Dir", desc.Dir);
		material->SetColor("DirLight_Diffuse", desc.Color);

		break;
	case LightType_Point:

		break;
	}
}

void engine::Light::Destroy()
{
	m_bDestroyed = true;
}

void engine::Light::registerComponent(ApplicationMode mode)
{
	if (mode == CLIENT)
	{
		RenderManager::GetInstance().AddLight(std::static_pointer_cast<Light>(shared_from_this()));
	}

	if (mode == EDITOR)
	{
		editor::EditorComponentManager::GetInstance().AddComponent(std::static_pointer_cast<Light>(shared_from_this()));
	}
}
