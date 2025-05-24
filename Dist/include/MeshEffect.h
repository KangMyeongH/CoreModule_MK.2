#pragma once
#include "Effect.h"
#include "LoadManager.h"
#include "Material.h"

namespace engine
{
    class COREMODULE_API MeshEffect : public Effect
    {
        //======================================//
        //				constructor				//
        //======================================//
    protected:
        explicit MeshEffect(const SharedPtr<GameObject>& owner, const _string& name = "MeshEffect");
        ~MeshEffect() override = default;
        MeshEffect(const MeshEffect& rhs);

        //======================================//
        //				 property				//
        //======================================//

        //======================================//
        //				  method				//
        //======================================//
    public:
        void InputAssembler(ID3D11DeviceContext* context) override;
        void Bind(ID3D11DeviceContext* context) override;
        void Render(ID3D11DeviceContext* context) override;

        SharedPtr<Component> Clone() const override;
        void Destroy() override;

    protected:
        void registerComponent(ApplicationMode mode = CLIENT) override;

    private:
        static SharedPtr<MeshEffect> create()
        {
            const auto& meshEffect = SharedPtr<MeshEffect>(new MeshEffect(nullptr), []
            (const MeshEffect* ptr) { delete ptr; });

            meshEffect->m_Material = Material::Create(meshEffect);

            return meshEffect;
        }

        //======================================//
        //				 serialize				//
        //======================================//
    public:
        void to_json(nlohmann::ordered_json& j) override
        {
            _string type = "MeshEffect";
            j = nlohmann::ordered_json{
				{"type", type},
				{"enable", m_bEnabled},
				{"modelPath", m_ModelPath},
				{"material", m_Material->GetPath()},
				{"wrap", m_bWrap},
				{"clamp", m_bClamp}
            };

        }

        void from_json(const nlohmann::ordered_json& j) override
        {
	        if (j.contains("enable"))
	        {
                j.at("enable").get_to(m_bEnabled);
	        }

            if (j.contains("modelPath"))
            {
                j.at("modelPath").get_to(m_ModelPath);
                SetMesh(m_ModelPath);
            }

            if (j.contains("material"))
            {
                _wstring path = j.at("material").get<_wstring>();
                m_Material = Material::Create(shared_from_this());
                LoadManager::GetInstance().LoadMaterialData(m_Material, path);
            }

            if (j.contains("wrap"))
            {
                j.at("wrap").get_to(m_bWrap);
            }

            if (j.contains("clamp"))
            {
                j.at("clamp").get_to(m_bClamp);
            }
        }

        //======================================//
        //				  fields				//
        //======================================//
    private:
        static ComponentRegistrar registrar_MeshEffect;
    };
}
