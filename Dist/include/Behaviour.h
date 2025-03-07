#pragma once
#include "Component.h"
#include "GameObject.h"

namespace engine
{
    class COREMODULE_API Behaviour : public Component
    {
    protected:
		explicit Behaviour(const SharedPtr<GameObject>& owner) : Component(owner), m_bEnabled(true) {}
		~Behaviour() override = default;

	    Behaviour(const Behaviour& rhs)
			: Component(rhs), m_bEnabled(rhs.m_bEnabled)
		{}

    public:
		virtual void 	SetEnable(const bool enabled) { m_bEnabled = enabled; }
		bool			IsEnabled() const;

		void 					Destroy() override = 0;
		SharedPtr<Component> 	Clone() const override = 0;
		void 					to_json(nlohmann::ordered_json& j) override = 0;
		void 					from_json(const nlohmann::ordered_json& j) override = 0;

	protected:
		void registerComponent() override = 0;

		bool m_bEnabled;
    };
}
