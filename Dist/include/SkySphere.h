#pragma once
#include "Object.h"

namespace engine
{
	class Mesh;

	class COREMODULE_API SkySphere : public Object
    {
        //======================================//
        //				constructor				//
        //======================================//
    protected:
        explicit SkySphere(const _string& name = "SkySphere");
        ~SkySphere() override = default;
        SkySphere(const SkySphere& rhs);

        //======================================//
        //				 property				//
        //======================================//


        //======================================//
        //				  method				//
        //======================================//
	public:
        void Initialize(const ComPtr<ID3D11Device>& device, const ComPtr<ID3D11DeviceContext>& context);

        void Render(const ComPtr<ID3D11DeviceContext>& context, const _float4X4& view, const _float4X4& proj, const Vector3& camPos, const _float3& sunDir);

        static SharedPtr<SkySphere> Create();

        void Destroy() override {}

        //======================================//
        //				 serialize				//
        //======================================//

        //======================================//
        //				  fields				//
        //======================================//
	private:
        ComPtr<ID3D11SamplerState> m_Sampler;

        SharedPtr<Material>     m_Material;
        SharedPtr<Mesh>         m_Mesh;

        Quaternion m_Rotation;
        Vector3 m_Scale;

        _float3 m_ColorA;
        _float3 m_ColorB;
        _float3 m_HorizonColor;
        _float3 m_ZenithColor;
        _float2 m_ScrollSpeed;
        _float  m_Time;
    };
}
