#include "EditorCamera.h"

engine::editor::EditorCamera::EditorCamera()
	: m_Position(0.f, 0.f, 0.f), m_Rotation(0.f, 0.f, 0.f),
	m_Target(0.f, 2.f, 0.f), m_Up(0.f, 1.f, 0.f),
	m_FiledOfView(0), m_AspectRatio(0), m_NearPlane(0), m_FarPlane(0)
{
}
