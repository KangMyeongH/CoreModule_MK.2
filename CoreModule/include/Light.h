#pragma once
#include "Behaviour.h"

namespace engine
{
    class COREMODULE_API Light final : public Behaviour
    {
        DECLARE_REGISTER_COMPONENT(Light)
        //======================================//
        //				constructor				//
        //======================================//
    protected:
        explicit Light(const SharedPtr<GameObject>& owner, const _string& name = "Light");
        ~Light() override;
        Light(const Light& rhs);

        //======================================//
        //				 property				//
        //======================================//
    public:
        void SetType(const LightType type) { m_Type = type; }
        LightType GetType() const { return m_Type; }

        void SetColor(const _float4& color) { m_Color = color; }
        _float4 GetColor() const { return m_Color; }

        void SetIntensity(const _float intensity) { m_Intensity = intensity; }
        _float GetIntensity() const { return m_Intensity; }

        void SetRange(const _float range) { m_Range = range; }
        _float GetRange() const { return m_Range; }

        void SetSpotAngle(const _float spotAngle) { m_SpotAngle = spotAngle; }
        _float GetSpotAngle() const { return m_SpotAngle; }

        LightDesc GetLightDesc() const;

        //======================================//
        //				  method				//
        //======================================//
    public:
        void BindLight(const SharedPtr<Material>& material);

        void Destroy() override;

    protected:
        void registerComponent(ApplicationMode mode = CLIENT) override;

        //======================================//
        //				 serialize				//
        //======================================//
    public:
        void to_json(nlohmann::ordered_json& j) override
        {
            _string type = "Light";
            j = nlohmann::ordered_json{
                {"type", type },
				{"enable", m_bEnabled},
				{"lightType", m_Type},
                {"colorR",m_Color.x},
                {"colorG", m_Color.y},
                {"colorB", m_Color.z},
                {"colorA", m_Color.w},
				{"intensity", m_Intensity},
				{"range", m_Range},
				{"spotAngle", m_SpotAngle}
            };
        }

        void from_json(const nlohmann::ordered_json& j) override
        {
	        if (j.contains("enable"))
	        {
                j.at("enable").get_to(m_bEnabled);
	        }

            if (j.contains("lightType"))
            {
                m_Type = j.at("lightType").get<LightType>();
            }

            if (j.contains("colorR"))
            {
                j.at("colorR").get_to(m_Color.x);
            }

            if (j.contains("colorG"))
            {
                j.at("colorG").get_to(m_Color.y);
            }

            if (j.contains("colorB"))
            {
                j.at("colorB").get_to(m_Color.z);
            }

            if (j.contains("colorA"))
            {
                j.at("colorA").get_to(m_Color.w);
            }

            if (j.contains("intensity"))
            {
                j.at("intensity").get_to(m_Intensity);
            }

            if (j.contains("range"))
            {
                j.at("range").get_to(m_Range);
            }

            if (j.contains("spotAngle"))
            {
                j.at("spotAngle").get_to(m_SpotAngle);
            }
        }

        //======================================//
        //				  fields				//
        //======================================//
    private:
        LightType   m_Type;
        _float4 	m_Color;
        _float 		m_Intensity;
        _float      m_Range;
        _float      m_SpotAngle;

    };
}
