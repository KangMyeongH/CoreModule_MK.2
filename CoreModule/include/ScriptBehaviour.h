#pragma once
#include "Behaviour.h"

namespace engine
{
    class COREMODULE_API ScriptBehaviour : public Behaviour
    {
    protected:
        explicit ScriptBehaviour(const SharedPtr<GameObject>& owner, const _string& name = "ScriptBehaviour")
	        : Behaviour(owner, name) {}

        ~ScriptBehaviour() override = default;

    public:
        virtual void Awake() {}
        virtual void OnEnable() {}
        virtual void Start() {}
        virtual void FixedUpdate() {}
        virtual void Update() {}
        virtual void LateUpdate() {}
        virtual void OnDestroy() {}
        virtual void OnDisable() {}

        //virtual void OnCollisionEnter(Collision other) {}
        //virtual void OnCollisionStay(Collision other) {}
        //virtual void OnCollisionExit(Collision other) {}

        void SetEnable(_bool enabled) final;
        void Destroy() final;

        SharedPtr<Component> Clone() const override = 0;

        void to_json(nlohmann::ordered_json& j) override = 0;
        void from_json(const nlohmann::ordered_json& j) override = 0;

    protected:
        void registerComponent() final;
    };
}
