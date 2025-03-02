#pragma once
#include "core_defines.h"

namespace engine
{
    class COREMODULE_API InputManager
    {
    private:
        //======================================//
        //				constructor				//
        //======================================//

        InputManager();
        ~InputManager();
    public:
        DECLARE_SINGLETON(InputManager)

        //======================================//
        //				 property				//
        //======================================//

        Vector3 GetMousePos() const;
        void    SetMousePos(const Vector3& mousePos);

        //======================================//
        //				  method				//
        //======================================//

        void 	UpdateInput(HWND hwnd);
        _bool   IsKeyPressed(_int key) const;
        _bool   IsKeyPressing(_int key) const;
        _bool   IsKeyDown(_int key) const;
        _bool   IsKeyUp(_int key) const;

    private:
        enum KEY_STATE
        {
	        KEY_IDLE = 0b0000,          // 눌리지 않은 상태
            KEY_PRESSING = 0b0001,      // 눌리고 있는 상태
            KEY_DOWN = 0b0010,          // 이전에 눌리지 않고, 이번에 눌린 상태
            KEY_UP = 0b0100             // 이전에 눌렸는데 이번에 안눌린 상태
        };
        KEY_STATE   m_KeyState[VK_MAX];
        Vector3     m_MousePos;
    };
}
