#pragma once
#include "core_defines.h"

namespace engine
{
    class Camera;
    class Renderer;

    using Renderers = std::vector<SharedPtr<Renderer>>;
    using RendererList = std::list<SharedPtr<Renderer>>;
    using Cameras = std::list<SharedPtr<Camera>>;
    using Models = std::unordered_map<_wstring, ModelData>;

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

        void                AddRenderer(const SharedPtr<Renderer>& renderer);

        //======================================//
        //				  method				//
        //======================================//

        void UpdateMainCamera();

    	void Render(const ComPtr<ID3D11DeviceContext>& context);

        void RegisterRenderer();
        void FlushDestroyRenderer();

        void FlushDestroyCamera();

        void Release();

    private:
        //======================================//
        //				  fields				//
        //======================================//

        Renderers           m_Renderers;
        RendererList        m_RegisterQueue;

    	WeakPtr<Camera> 	m_MainCamera;
        Cameras 			m_Cameras;
        Models              m_Models;

        _float4X4           m_ViewMat;
        _float4X4           m_ProjMat;
    };
}
