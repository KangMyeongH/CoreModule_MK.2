#pragma once
#include "core_defines.h"

namespace engine
{
    class CapsuleRenderer
    {
        //======================================//
        //				constructor				//
        //======================================//
    public:
        CapsuleRenderer();
        ~CapsuleRenderer();

        //======================================//
        //				 property				//
        //======================================//



        //======================================//
        //				  method				//
        //======================================//
    public:
        HRESULT Initialize(const ComPtr<ID3D11Device>& device);

        void Bind(const ComPtr<ID3D11DeviceContext>& context, const _float4X4& view, const _float4X4& proj);

        void Render(const ComPtr<ID3D11DeviceContext>& context, const _vector& center, const Capsule& capsule);

    private:
        void AddLine(const _float3& a, const _float3& b, const _float4& color);
        HRESULT CreateDynamicVB(const ComPtr<ID3D11Device>& device, UINT maxVerts);
        HRESULT compileShaderFromFile(const _wstring& path, const _string& entryPoint, const _string& targetProfile, ComPtr<ID3DBlob>& outBlob);

        //======================================//
        //				 serialize				//
        //======================================//

        //======================================//
        //				  fields				//
        //======================================//
    private:
        ComPtr<ID3D11Buffer>        m_VTXBuffer;

        SharedPtr<Material>           m_Material;

        std::vector<DebugVertex> m_Batch;
    };
}
