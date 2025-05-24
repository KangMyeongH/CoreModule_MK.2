#pragma once
#include "Behaviour.h"

namespace engine
{
	class Mesh;

	class COREMODULE_API Effect : public Behaviour
    {
        //======================================//
        //				constructor				//
        //======================================//
	protected:
        explicit Effect(const SharedPtr<GameObject>& owner, const _string& name = "Effect");
        ~Effect() override = default;
        Effect(const Effect& rhs);

        //======================================//
        //				 property				//
        //======================================//
	public:
		void SetMesh(const _wstring& modelPath);
        SharedPtr<Mesh> GetMesh() const { return m_Mesh; }

        void SetMaterial(const _wstring& materialPath) const;
        void SetMaterial(const SharedPtr<Material>& material);
        SharedPtr<Material> GetMaterial() const { return m_Material; }

        _bool IsWrap() const { return m_bWrap; }
        void SetWrap();

        _bool IsClamp() const { return m_bClamp; }
        void SetClamp();

        //======================================//
        //				  method				//
        //======================================//
	public:
        virtual void InputAssembler(ID3D11DeviceContext* context) = 0;
        virtual void Bind(ID3D11DeviceContext* context) = 0;
        virtual void Render(ID3D11DeviceContext* context) = 0;



	protected:
        void registerComponent(ApplicationMode mode = CLIENT) override;

        //======================================//
        //				 serialize				//
        //======================================//
        void to_json(nlohmann::ordered_json& j) override = 0;
        void from_json(const nlohmann::ordered_json& j) override = 0;

        //======================================//
        //				  fields				//
        //======================================//
	protected:
        SharedPtr<Mesh> m_Mesh;
        SharedPtr<Material> m_Material;
        _bool   m_bWrap;
        _bool   m_bClamp;

        _wstring m_ModelPath;
    };
}
