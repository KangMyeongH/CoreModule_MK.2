#pragma once
#include "LoadManager.h"
#include "Material.h"
#include "UI.h"

namespace engine
{
    class COREMODULE_API EffectUI : public UI
    {
        DECLARE_REGISTER_COMPONENT(EffectUI)
        //======================================//
        //				constructor				//
        //======================================//
    protected:
        explicit EffectUI(const SharedPtr<GameObject>& owner, const _string& name = "EffectUI");
        ~EffectUI() override = default;
        EffectUI(const EffectUI& rhs);

        //======================================//
        //				 property				//
        //======================================//
    public:
        _wstring GetMaterialPath() const { return m_MaterialPath; }

        //======================================//
        //				  method				//
        //======================================//
    public:
        _bool IsMouseHovered() override { return false; }
        _bool IsButtonDown() override { return false; }
        _bool IsButtonHold() override { return false; }
        _bool IsButtonUp() override { return false; }

        void Update() override;
        HRESULT InputAssembler(const ComPtr<ID3D11DeviceContext>& context) override;
        void RenderUI(const ComPtr<ID3D11DeviceContext>& context) override;

        void Destroy() override;

    protected:
        void registerComponent(ApplicationMode mode) override;

        //======================================//
        //				 serialize				//
        //======================================//
    public:
        void to_json(nlohmann::ordered_json& j) override
        {
            _string type = "EffectUI";
            j = nlohmann::ordered_json{
                {"type", type},
                {"enable", m_bEnabled},
                {"materialPath", m_Material->GetPath()},
                {"textureWidth", m_TextureSize.x},
                {"textureHeight", m_TextureSize.y}
            };
        }

        void from_json(const nlohmann::ordered_json& j) override
        {
	        if (j.contains("enable"))
	        {
                j.at("enable").get_to(m_bEnabled);
	        }

            if (j.contains("materialPath"))
            {
                j.at("materialPath").get_to(m_MaterialPath);
                m_Material = Material::Create(shared_from_this());
                LoadManager::GetInstance().LoadMaterialData(m_Material, m_MaterialPath);
            }

            if (j.contains("textureWidth"))
            {
                j.at("textureWidth").get_to(m_TextureSize.x);
            }

            if (j.contains("textureHeight"))
            {
                j.at("textureHeight").get_to(m_TextureSize.y);
            }
        }

        //======================================//
        //				  fields				//
        //======================================//
    private:
        ComPtr<ID3D11SamplerState> m_Sampler;
        _wstring        m_MaterialPath;
        _float2         m_TextureSize;
    };
}
