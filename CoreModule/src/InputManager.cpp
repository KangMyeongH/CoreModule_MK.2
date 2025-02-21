#include "InputManager.h"

IMPLEMENT_SINGLETON(engine::InputManager)

engine::InputManager::InputManager()
{
	memset(m_KeyState, KEY_IDLE, sizeof(m_KeyState));
}

engine::InputManager::~InputManager() = default;

engine::Vector3 engine::InputManager::GetMousePos() const
{
	return m_MousePos;
}

void engine::InputManager::SetMousePos(const Vector3& mousePos)
{
	m_MousePos = mousePos;
}

engine::_bool engine::InputManager::IsKeyPressed(const _int key) const
{
	if (m_KeyState[key] & (KEY_PRESSING | KEY_DOWN))
	{
		return true;
	}

	return false;
}

engine::_bool engine::InputManager::IsKeyPressing(const _int key) const
{
	if (m_KeyState[key] == KEY_PRESSING)
	{
		return true;
	}

	return false;
}

engine::_bool engine::InputManager::IsKeyDown(const _int key) const
{
	if (m_KeyState[key] == KEY_DOWN)
	{
		return true;
	}

	return false;
}

engine::_bool engine::InputManager::IsKeyUp(const _int key) const
{
	if (m_KeyState[key] == KEY_UP)
	{
		return true;
	}

	return false;
}

void engine::InputManager::UpdateInput(HWND hwnd)
{
	for (_int i = 0; i < VK_MAX; ++i)
	{
		if (m_KeyState[i] & (KEY_PRESSING | KEY_DOWN))
		{
			if (GetAsyncKeyState(i) & 0x8000)
			{
				m_KeyState[i] = KEY_PRESSING;
			}

			else
			{
				m_KeyState[i] = KEY_UP;
			}
		}

		else
		{
			if (GetAsyncKeyState(i) & 0x8000)
			{
				m_KeyState[i] = KEY_DOWN;
 			}

			else
			{
				m_KeyState[i] = KEY_IDLE;
			}
		}
	}

	POINT mouse;
	GetCursorPos(&mouse);
	ScreenToClient(hwnd, &mouse);
	// m_MousePos = Vector3{ static_cast<_float>(mouse.x), static_cast<_float>(mouse.y), 0.f };
}

