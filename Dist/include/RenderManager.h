#pragma once
#include "core_defines.h"

namespace engine
{
    class Camera;
    class Renderer;
    class Light;
    class SkySphere;

    using Renderers = std::vector<SharedPtr<Renderer>>;
    using RendererList = std::list<SharedPtr<Renderer>>;
    using Cameras = std::list<SharedPtr<Camera>>;
    using Models = std::unordered_map<_wstring, ModelData>;
    using Lights = std::vector<SharedPtr<Light>>;
    using LightList = std::list<SharedPtr<Light>>;

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
        void                Initialize(const ComPtr<ID3D11Device>& device, const ComPtr<ID3D11DeviceContext>& context);


        void                SetMainCamera(const SharedPtr<Camera>& camera) { m_MainCamera = camera; }
        SharedPtr<Camera> 	GetMainCamera() const { return m_MainCamera.lock(); }

        _float4X4           GetViewMat() const { return m_ViewMat; }
        _float4X4           GetProjMat() const { return m_ProjMat; }

        SharedPtr<SkySphere> GetSkySphere() const { return m_SkySphere; }

        void                AddCamera(const SharedPtr<Camera>& camera);

        void                AddRenderer(const SharedPtr<Renderer>& renderer);

        void                AddLight(const SharedPtr<Light>& light);

        //======================================//
        //				  method				//
        //======================================//

        void UpdateMainCamera();

        void RenderSkySphere(const ComPtr<ID3D11DeviceContext>& context);
    	void Render(const ComPtr<ID3D11DeviceContext>& context);

        void RegisterRenderer();
        void FlushDestroyRenderer();

        void RegisterLight();
        void FlushDestroyLight();

        void FlushDestroyCamera();

        void Release();

    private:
        //======================================//
        //				  fields				//
        //======================================//

        Renderers           m_Renderers;
        RendererList        m_RegisterQueue;

        Lights              m_Lights;
        LightList           m_LightRegisterQueue;

    	WeakPtr<Camera> 	m_MainCamera;
        Cameras 			m_Cameras;
        Models              m_Models;

        SharedPtr<SkySphere> 	m_SkySphere;
        _float4             	m_AmbientColor;

        _float4X4           m_ViewMat;
        _float4X4           m_ProjMat;
    };
}
