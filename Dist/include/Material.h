#pragma once
#include "Object.h"

namespace engine
{
    class Renderer;

    class Material : public Object
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



        static SharedPtr<Material> Create(const SharedPtr<Renderer>& renderer);
	    void Destroy() override;

    private:
        WeakPtr<Renderer> 			m_Owner;
        ID3D11ShaderResourceView* 	m_DiffuseTexture;
        _float4                     m_DiffuseColor;
        _float4                     m_AmbientColor;
    };
}
