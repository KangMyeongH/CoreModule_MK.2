#pragma once
#include "core_defines.h"

namespace engine
{
    class COREMODULE_API DebugRenderManager
    {
        //======================================//
        //				constructor				//
        //======================================//
    private:
        DebugRenderManager();
        ~DebugRenderManager();
    public:
        DECLARE_SINGLETON(DebugRenderManager)

        //======================================//
        //				 property				//
        //======================================//

        //======================================//
        //				  method				//
        //======================================//
    public:
        void Initialize(const ComPtr<ID3D11Device>& device);

    	void RenderCollider(const std::vector<SharedPtr<Collider>>& colliders, const ComPtr<ID3D11DeviceContext>& context, const _float4X4& viewMat, const _float4X4& projMat);

        void Release();

    private:
        HRESULT createOBBWireFrameVertices(const ComPtr<ID3D11Device>& device);
        HRESULT createSphereWireframe(const ComPtr<ID3D11Device>& device);
        HRESULT createCapsuleWireframe(const ComPtr<ID3D11Device>& device);

        //======================================//
        //				 serialize				//
        //======================================//

        //======================================//
        //				  fields				//
        //======================================//
    private:
        SharedPtr<VIBuffer> m_BoxVIBuffer;
        SharedPtr<VIBuffer> m_SphereVIBuffer;
        SharedPtr<VIBuffer> m_CapsuleVIBuffer;
        SharedPtr<Material> m_ColliderMaterial;
    };
}
