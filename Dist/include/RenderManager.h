#pragma once
#include "core_defines.h"

namespace engine
{
    class Camera;

    using Cameras = std::list<SharedPtr<Camera>>;

    class COREMODULE_API RenderManager
    {
    private:
        //======================================//
        //				constructor				//
        //======================================//

        RenderManager();
        ~RenderManager();
    public:
        DECLARE_SINGLETON(RenderManager)

        //======================================//
        //				 property				//
        //======================================//

        void                SetMainCamera(const SharedPtr<Camera>& camera) { m_MainCamera = camera; }
        SharedPtr<Camera> 	GetMainCamera() const { return m_MainCamera.lock(); }

        void                AddCamera(const SharedPtr<Camera>& camera);

        //======================================//
        //				  method				//
        //======================================//

        void UpdateMainCamera();
        void Render();

        void FlushDestroyCamera();

        void Release();

    private:
        WeakPtr<Camera> 	m_MainCamera;
        Cameras 			m_Cameras;

        VS_ConstantBuffer   m_VSConstantBuffer;

    };
}
