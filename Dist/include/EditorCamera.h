#pragma once
#include "core_defines.h"

namespace engine
{
    namespace editor
    {
        class COREMODULE_API EditorCamera
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
            Vector3 GetCameraPos() const { return m_Position; }

            void 	SetAspectRatio(const _float aspectRatio) { m_AspectRatio = aspectRatio; }
            _float 	GetAspectRatio() const { return m_AspectRatio; }


            //======================================//
            //				  method				//
            //======================================//
        public:
        	_matrix GetViewMatrix() const;
            _matrix GetProjectMatrix() const;

            void 	MoveInViewDir(_float forwardAmount, _float rightAmount);
            void	Move(const Vector3& dir, _float distance);
            void 	RotateInView(_float xAmount, _float yAmount);
            void 	Rotate(_float y, _float x);
            void 	Zoom(_float delta);

            //======================================//
            //				 serialize				//
            //======================================//

            //======================================//
            //				  fields				//
            //======================================//
        private:
            Vector3 	m_Position;
            Quaternion 	m_Rotation;

            _float	m_FiledOfView;
            _float	m_AspectRatio;
            _float	m_NearPlane;
            _float	m_FarPlane;
        };
    }
}