#include "Camera.h"

#include "core_struct.h"
#include "RenderManager.h"
#include "Transform.h"

DEFINE_REGISTER_COMPONENT(Camera)

engine::Camera::Camera(const SharedPtr<GameObject>& owner)
	: Component(owner),
	m_ViewMat(), m_ProjMat(),
    m_FiledOfView(90.f), m_AspectRatio(16.f/9.f),
    m_NearPlane(1), m_FarPlane(100)
{

}

engine::Camera::Camera(const Camera& rhs)
	: Component(rhs),
	m_ViewMat(rhs.m_ViewMat), m_ProjMat(rhs.m_ProjMat),
	m_FiledOfView(rhs.m_FiledOfView), m_AspectRatio(16.f / 9.f),
	m_NearPlane(rhs.m_NearPlane), m_FarPlane(rhs.m_FarPlane)
{
	
}

void engine::Camera::UpdateCamera(_float4X4& viewMat, _float4X4& projMat) const
{
	_matrix rotMat = DirectX::XMMatrixRotationQuaternion(GetTransform()->Rotation().ToVector());

	_vector eye = GetTransform()->Position().ToVector();
	_vector defaultForward = DirectX::XMVectorSet(0.f, 0.f, 1.f, 0.f);
	_vector defaultUp = DirectX::XMVectorSet(0.f, 1.f, 0.f, 0.f);

	_vector forward = XMVector3TransformNormal(defaultForward, rotMat);
	_vector up 		= XMVector3TransformNormal(defaultUp, rotMat);

	forward = DirectX::XMVector3Normalize(forward);
	up 		= DirectX::XMVector3Normalize(up);

	const _matrix viewMatrix = DirectX::XMMatrixLookToLH(eye, forward, up);
	const _matrix projMatrix = DirectX::XMMatrixPerspectiveFovLH(m_FiledOfView, m_AspectRatio, m_NearPlane, m_FarPlane);

	XMStoreFloat4x4(&viewMat, viewMatrix);
	XMStoreFloat4x4(&projMat, projMatrix);
}

void engine::Camera::SetMainCamera()
{
	RenderManager::GetInstance().SetMainCamera(std::static_pointer_cast<Camera>(shared_from_this()));
}

void engine::Camera::Destroy()
{
	if (!m_bDestroyed)
	{
		m_bDestroyed = true;
	}
}

void engine::Camera::to_json(nlohmann::ordered_json& j)
{
	std::string type = "Camera";
	j = nlohmann::ordered_json{
		{"type", type},
		{"fov", m_FiledOfView},
		{"aspectRatio", m_AspectRatio},
		{"nearPlane", m_NearPlane},
		{"farPlane", m_FarPlane }
	};
}

void engine::Camera::from_json(const nlohmann::ordered_json& j)
{
	j.at("fov").get_to(m_FiledOfView);
	j.at("aspectRatio").get_to(m_AspectRatio);
	j.at("nearPlane").get_to(m_NearPlane);
	j.at("farPlane").get_to(m_FarPlane);
}

void engine::Camera::registerComponent(ApplicationMode mode)
{
	if (mode == CLIENT)
	{
		RenderManager::GetInstance().AddCamera(std::static_pointer_cast<Camera>(shared_from_this()));
	}

	else if (mode == EDITOR)
	{
		
	}
}

