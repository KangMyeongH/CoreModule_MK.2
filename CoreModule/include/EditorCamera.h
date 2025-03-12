#pragma once
#include "core_defines.h"

namespace engine
{
    namespace editor
    {
        class EditorCamera
        {
            //======================================//
            //				constructor				//
            //======================================//
        public:
            EditorCamera();
            ~EditorCamera() = default;

            //======================================//
            //				 property				//
            //======================================//
        public:


            //======================================//
            //				  method				//
            //======================================//

            //======================================//
            //				 serialize				//
            //======================================//

            //======================================//
            //				  fields				//
            //======================================//
        private:
            Vector3 m_Position;
            Vector3 m_Rotation;
            Vector3 m_Target;
            Vector3 m_Up;

            _float	m_FiledOfView;
            _float	m_AspectRatio;
            _float	m_NearPlane;
            _float	m_FarPlane;
        };
    }
}