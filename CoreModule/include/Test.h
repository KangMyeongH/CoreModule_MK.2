#pragma once
#include "ScriptBehaviour.h"

namespace engine
{
    class Test: public ScriptBehaviour
    {
        DECLARE_REGISTER_COMPONENT(Test)

    private:
        explicit Test(const SharedPtr<GameObject>& owner) : ScriptBehaviour(owner) {}
        ~Test() override = default;

    public:
        void to_json(nlohmann::ordered_json& j) override
        {
        }

        void from_json(const nlohmann::ordered_json& j) override
        {
        }
    };
}
