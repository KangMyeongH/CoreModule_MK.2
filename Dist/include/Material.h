#pragma once
#include "Object.h"

namespace engine
{
    class Renderer;

    class COREMODULE_API Material : public Object
    {
    protected:
        //======================================//
        //				constructor				//
        //======================================//

        explicit Material(const SharedPtr<Renderer>& owner, const _string& name = "Material");
        ~Material() override;
    	Material(const Material& rhs);

    public:
        //======================================//
        //				 property				//
        //======================================//

        void SetFloat(const _string& name, _float value);
        _float GetFloat(const _string& name);

        void SetFloat2(const _string& name, _float2 value);
        _float2 GetFloat2(const _string& name);

        void SetFloat3(const _string& name, _float3 value);
        _float3 GetFloat3(const _string& name);

        void SetFloat4(const _string& name, _float4 value);
        _float4 GetFloat4(const _string& name);

        void SetMatrix(const _string& name, const _float4X4& value);
        _float4X4 GetMatrix(const _string& name);

        void SetColor(const _string& name, _float4 value);
        _float4 GetColor(const _string& name);

        void SetTexture(const _string& name, const ComPtr<ID3D11ShaderResourceView>& texture);
        void SetTexture(const _string& name, const _wstring& path);

        void SetSampler(const _string& name, const ComPtr<ID3D11SamplerState>& sampler);

        void SetOwner(const SharedPtr<Renderer>& renderer) { m_Owner = renderer; }
    	SharedPtr<Renderer> GetOwner() const { return m_Owner.lock(); }

        SharedPtr<Shader> GetShader() const { return m_Shader; }

        //======================================//
        //				  method				//
        //======================================//

        void LoadShader(const _wstring& path);

        void Bind(ID3D11DeviceContext* context) const;

        static SharedPtr<Material> Create(const SharedPtr<Renderer>& renderer);

	    void Destroy() override;

    private:
        //======================================//
        //				  fields				//
        //======================================//

        WeakPtr<Renderer> 					m_Owner;
        SharedPtr<Shader>           		m_Shader;
        _wstring                            m_ShaderPath;
    };
}
