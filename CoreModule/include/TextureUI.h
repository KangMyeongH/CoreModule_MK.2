#pragma once
#include "UI.h"

namespace engine
{
    class COREMODULE_API TextureUI : public UI
    {
        DECLARE_REGISTER_COMPONENT(TextureUI)

    protected:
	    //======================================//
        //				constructor				//
        //======================================//

        explicit TextureUI(const SharedPtr<GameObject>& owner);
        ~TextureUI() override = default;
        TextureUI(const TextureUI& rhs);

    public:
        //======================================//
        //				 property				//
        //======================================//

        void SetTexture(const _wstring& path);
        ComPtr<ID3D11ShaderResourceView> GetTexture() const;

    public:
        //======================================//
        //				  method				//
        //======================================//

        _bool IsMouseHovered() override;
        _bool IsButtonDown() override;
        _bool IsButtonHold() override;
        _bool IsButtonUp() override;

        void Update() override;
        void RenderUI() override;

        void Destroy() override;

    public:
        //======================================//
        //				 serialize				//
        //======================================//

        void to_json(nlohmann::ordered_json& j) override;
        void from_json(const nlohmann::ordered_json& j) override;

    private:
        //======================================//
        //				  fields				//
        //======================================//

        ComPtr<ID3D11ShaderResourceView>    m_Texture;
        _float4X4                           m_TextureScaleMatrix;
        _wstring                            m_Path;
        _uint                               m_Width;
        _uint                               m_Height;
        _bool                               m_bFlipX;
        _bool                               m_bFlipY;
    };
}
