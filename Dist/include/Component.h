#pragma once
#include "Object.h"
#include "ComponentRegistrar.h"
#include "ComponentFactory.h"

namespace engine
{
    class GameObject;
    class Transform;

    class COREMODULE_API Component : public Object
    {
        friend class GameObject;
        friend class ComponentRegister;
        friend class ::ComponentFactory;

    protected:
        explicit Component(const SharedPtr<GameObject>& owner) : m_Owner(owner)
        {
	        
        }

        ~Component() override = default;

    	Component(const Component& rhs) : Object(rhs)
        {
            m_Owner.reset();
        }

        Component& operator=(const Component& rhs)
        {
            if (this != &rhs)
            {
                Object::operator=(rhs);
                m_Owner = rhs.m_Owner;
            }

            return *this;
        }

        Component(Component&& rhs) noexcept : Object(std::move(rhs)), m_Owner(std::move(rhs.m_Owner))
    	{
            rhs.m_Owner.reset();
    	}

        Component& operator=(Component&& rhs) noexcept
    	{
    		if (this != &rhs)
    		{
                Object::operator=(std::move(rhs));
                m_Owner = rhs.m_Owner;
                rhs.m_Owner.reset();
    		}
            return *this;
    	}

    public:
    	WeakPtr<GameObject> 				GetGameObject() const { return m_Owner; }
        void        						SetOwner(const SharedPtr<GameObject>& owner) { m_Owner = owner; }
		SharedPtr<Transform> 				GetTransform() const;

        virtual SharedPtr<Component> 		Clone() const = 0;

        virtual void 						to_json(nlohmann::ordered_json& j) = 0;
        virtual void 						from_json(const nlohmann::ordered_json& j) = 0;

    protected:
        virtual void                        registerComponent() = 0;

    protected:
        WeakPtr<GameObject> 				m_Owner;

    };
}
