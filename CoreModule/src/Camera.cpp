#include "Camera.h"

#include "core_struct.h"
#include "RenderManager.h"
#include "Transform.h"

DEFINE_REGISTER_COMPONENT(Camera)

engine::Camera::Camera(const SharedPtr<GameObject>& owner)
	: Component(owner),
	m_ViewMat(), m_ProjMat(),
	m_Target(0.f, 0.f, -10.f), m_Up(0.f, 1.f, 0.f),
    m_FiledOfView(90.f), m_AspectRatio(),
    m_NearPlane(0), m_FarPlane(0)
{

}

engine::Camera::Camera(const Camera& rhs)
	: Component(rhs),
	m_ViewMat(rhs.m_ViewMat), m_ProjMat(rhs.m_ProjMat),
	m_Target(rhs.m_Target), m_Up(rhs.m_Up),
	m_FiledOfView(rhs.m_FiledOfView), m_AspectRatio(rhs.m_AspectRatio),
	m_NearPlane(rhs.m_NearPlane), m_FarPlane(rhs.m_FarPlane)
{
	
}

void engine::Camera::UpdateCamera(VS_ConstantBuffer* constantBuffer) const
{
	_matrix rotMat = DirectX::XMMatrixRotationQuaternion(GetTransform()->GetLocalRotation().ToVector());

	_vector eye = GetTransform()->GetLocalPosition().ToVector();
	_vector defaultForward = DirectX::XMVectorSet(0.f, 0.f, 1.f, 0.f);
	_vector defaultUp = DirectX::XMVectorSet(0.f, 1.f, 0.f, 0.f);

	_vector forward = XMVector3TransformNormal(defaultForward, rotMat);
	_vector up 		= XMVector3TransformNormal(defaultUp, rotMat);

	forward = DirectX::XMVector3Normalize(forward);
	up 		= DirectX::XMVector3Normalize(up);

	const _matrix viewMat = DirectX::XMMatrixLookToLH(eye, forward, up);
	const _matrix projMat = DirectX::XMMatrixPerspectiveFovLH(m_FiledOfView, m_AspectRatio, m_NearPlane, m_FarPlane);

	XMStoreFloat4x4(&constantBuffer->ViewMat, viewMat);
	XMStoreFloat4x4(&constantBuffer->ProjMat, projMat);
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
		{"instanceID", GetInstanceID()},
		{"fov", m_FiledOfView},
		{"aspectRatio", m_AspectRatio},
		{"nearPlane", m_NearPlane},
		{"farPlane", m_FarPlane }
	};
}

void engine::Camera::from_json(const nlohmann::ordered_json& j)
{
	SetInstanceID(j.at("instanceID").get<int>());
	j.at("fov").get_to(m_FiledOfView);
	j.at("aspectRatio").get_to(m_AspectRatio);
	j.at("nearPlane").get_to(m_NearPlane);
	j.at("farPlane").get_to(m_FarPlane);
}

void engine::Camera::registerComponent()
{
	RenderManager::GetInstance().AddCamera(std::static_pointer_cast<Camera>(shared_from_this()));
}

