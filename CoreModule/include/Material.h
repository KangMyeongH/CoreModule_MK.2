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

        explicit Material(const SharedPtr<Renderer>& owner);
        ~Material() override;
    	Material(const Material& rhs);

    public:
        //======================================//
        //				 property				//
        //======================================//

        void SetDiffuseTexture(const _wstring& path);

        void SetFloat(const std::string& name, _float value);
        _float GetFloat(const std::string& name);

        void SetFloat2(const std::string& name, _float2 value);
        _float2 GetFloat2(const std::string& name);

        void SetFloat3(const std::string& name, _float3 value);
        _float3 GetFloat3(const std::string& name);

        void SetFloat4(const std::string& name, _float4 value);
        _float4 GetFloat4(const std::string& name);

        void SetMatrix(const std::string& name, const _float4X4& value);
        _float4X4 GetMatrix(const std::string& name);

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

        WeakPtr<Renderer> 			m_Owner;
        SharedPtr<Shader>           m_Shader;

    	ID3D11ShaderResourceView* 	m_DiffuseTexture;

        std::unordered_map<UINT, std::vector<uint8_t>> m_CBufferData;
        std::unordered_map<UINT, ID3D11Buffer*> m_CBufferObjects;
    };
}
