#include "EditorCamera.h"

engine::editor::EditorCamera::EditorCamera()
	: m_Position(0.f, 2.f, -10.f), m_Rotation(0.f, 0.f, 0.f, 1.f),
	m_FiledOfView(PI/4), m_AspectRatio(1.6f), m_NearPlane(1.0f), m_FarPlane(1000.f)
{
}

engine::_matrix engine::editor::EditorCamera::GetViewMatrix() const
{
	_matrix rotMat = DirectX::XMMatrixRotationQuaternion(m_Rotation.ToVector());

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
	_vector localForward = DirectX::XMVectorSet(0.f, 0.f, 1.f, 0.f);
	_vector rotatedForward = DirectX::XMVector3Rotate(localForward, m_Rotation.ToVector());
	_vector forward = DirectX::XMVector3Normalize(rotatedForward);

	_vector localRight = DirectX::XMVectorSet(1.f, 0.f, 0.f, 0.f);
	_vector rotatedRight = DirectX::XMVector3Rotate(localRight, m_Rotation.ToVector());
	_vector right = DirectX::XMVector3Normalize(rotatedRight);

	_vector vMovement = DirectX::XMVectorAdd(DirectX::XMVectorScale(forward, forwardAmount), DirectX::XMVectorScale(right, rightAmount));

	Vector3 movement = Vector3::FromVector(vMovement);

	m_Position += movement;
}

void engine::editor::EditorCamera::Move(const Vector3& dir, _float distance)
{
	Vector3 movement = dir * distance;
	m_Position += movement;
}

void engine::editor::EditorCamera::RotateInView(_float xAmount, _float yAmount)
{
	_vector qPitch = DirectX::XMQuaternionRotationAxis(DirectX::XMVectorSet(0.f, 1.f, 0.f, 0.f), xAmount); // XÃà È¸Àü
	_vector qYaw = DirectX::XMQuaternionRotationAxis(DirectX::XMVectorSet(1.f, 0.f, 0.f, 0.f), yAmount);

	m_Rotation = Quaternion::FromVector(DirectX::XMQuaternionMultiply(m_Rotation.ToVector(), DirectX::XMQuaternionMultiply(qPitch, qYaw)));

	_matrix rotMatrix = DirectX::XMMatrixRotationQuaternion(m_Rotation.ToVector());
	float yaw; 
	float roll;
	float pitch = asinf(-rotMatrix.r[2].m128_f32[1]); // -m13
	if (cosf(pitch) > 0.0001f) {
		yaw = atan2f(rotMatrix.r[2].m128_f32[0], rotMatrix.r[2].m128_f32[2]); // m11, m33
		roll = atan2f(rotMatrix.r[0].m128_f32[1], rotMatrix.r[1].m128_f32[1]); // m21, m22
	}
	else {
		// Áü¹ú¶ô(gimbal lock) »óÅÂ
		yaw = atan2f(-rotMatrix.r[1].m128_f32[0], rotMatrix.r[0].m128_f32[0]); // -m21, m11
		roll = 0;
	}

	roll = 0.f;
}

void engine::editor::EditorCamera::Rotate(_float y, _float x)
{

}

void engine::editor::EditorCamera::Zoom(float delta)
{
	_vector localForward = DirectX::XMVectorSet(0.f, 0.f, 1.f, 0.f);
	_vector rotatedForward = DirectX::XMVector3Rotate(localForward, m_Rotation.ToVector());
	_vector forward = DirectX::XMVector3Normalize(rotatedForward);

	_float length;
	DirectX::XMStoreFloat(&length, DirectX::XMVector3Length(forward));

	_float zoomAmount = delta * length * 0.f;
	if (length + zoomAmount > 1.f)
	{
		m_Position += Vector3::FromVector(forward) * (zoomAmount / length);
	}
}
