#include "EditorCamera.h"

engine::editor::EditorCamera::EditorCamera()
	: m_Position(0.f, 2.f, -10.f), m_Rotation(0.f, 0.f, 0.f),
	m_FiledOfView(90.f), m_AspectRatio(1.6f), m_NearPlane(1.0f), m_FarPlane(1000.f)
{
}

engine::_matrix engine::editor::EditorCamera::GetViewMatrix() const
{
	_matrix rotMat = DirectX::XMMatrixRotationQuaternion(Quaternion::Euler(m_Rotation).ToVector());

	_vector eye = m_Position.ToVector();
	_vector defaultForward = DirectX::XMVectorSet(0.f, 0.f, 1.f, 0.f);
	_vector defaultUp = DirectX::XMVectorSet(0.f, 1.f, 0.f, 0.f);

	_vector forward = XMVector3TransformNormal(defaultForward, rotMat);
	_vector up		= XMVector3TransformNormal(defaultUp, rotMat);

	forward = DirectX::XMVector3Normalize(forward);
	up 		= DirectX::XMVector3Normalize(up);

	return DirectX::XMMatrixLookToLH(eye, forward, up);
}

engine::_matrix engine::editor::EditorCamera::GetProjectMatrix() const
{
	return DirectX::XMMatrixPerspectiveFovLH(m_FiledOfView, m_AspectRatio, m_NearPlane, m_FarPlane);
}

void engine::editor::EditorCamera::MoveInViewDir(_float forwardAmount, _float rightAmount)
{

}

void engine::editor::EditorCamera::Move(const Vector3& dir, _float distance)
{
}

void engine::editor::EditorCamera::RotateInView(_float xAmount, _float yAmount)
{
}

void engine::editor::EditorCamera::Rotate(_float y, _float x)
{
}

void engine::editor::EditorCamera::Zoom(float delta)
{
}
