#pragma once
#include "Component.h"

namespace engine
{
	using Children = std::vector<SharedPtr<Transform>>;

    class COREMODULE_API Transform final : public Component
    {
		friend class GameObject;
		friend class Scene;
	protected:
		//======================================//
		//				constructor				//
		//======================================//

	    explicit Transform(const SharedPtr<GameObject>& owner)
			: Component(owner), m_WorldMatrix(), m_bDirty(true)
		{
		}

		~Transform() override = default;
		Transform(const Transform& rhs);

    public:
		//======================================//
		//				  method				//
		//======================================//

		void SetParent(const SharedPtr<Transform>& parent)
		{
			auto oldParent = m_Parent.lock();

			if (oldParent != parent)
			{
				if (oldParent != nullptr)
				{
					oldParent->detachChild(std::dynamic_pointer_cast<Transform>(shared_from_this()));
				}

				m_Parent = parent;

				// TODO : 아래 주석 해제 해야함.
				//m_LocalPosition = m_WorldPosition;
				//m_LocalRotation = m_WorldRotation;
				//m_LocalScale = m_WorldScale;

				if (parent != nullptr)
				{
					// TODO : 계층 구조 기준으로 즉 부모의 Transform 정보 기준으로 LocalSpace 연산을 해줘야함

					parent->addChild(std::dynamic_pointer_cast<Transform>(shared_from_this()));
				}
			}
		}

    private:
		void addChild(const SharedPtr<Transform>& child)
		{
			m_Children.push_back(child);
		}

		void detachChild(const SharedPtr<Transform>& child)
		{
			auto it = std::remove(m_Children.begin(), m_Children.end(), child);

			if (it != m_Children.end())
			{
				m_Children.erase(it, m_Children.end());
			}
		}

		void setDirty()
		{
			m_bDirty = true;

			if (!m_Children.empty())
			{
				for (auto& it : m_Children)
				{
					it->setDirty();
				}
			}
		}

		void registerComponent() override {}
		static SharedPtr<Transform> create();

    public:
	    void Destroy() override;
	    SharedPtr<Component> Clone() const override;

		void to_json(nlohmann::ordered_json& j) override;
		void from_json(const nlohmann::ordered_json& j) override;
		friend void to_json(nlohmann::ordered_json& j, const SharedPtr<Transform>& t);
		friend void from_json(const nlohmann::ordered_json& j, SharedPtr<Transform>& t);


	private:
		static std::unordered_map<int, SharedPtr<Transform>> s_TransformMap;
		static std::unordered_map<int, std::vector<SharedPtr<Transform>>> s_ChildTransformMap;

		WeakPtr<Transform>					m_Parent;
		std::vector<SharedPtr<Transform>> 	m_Children;

		mutable _float4X4	m_WorldMatrix;

		mutable Vector3		m_WorldPosition;
		mutable Quaternion	m_WorldRotation;
		mutable Vector3		m_WorldScale;

		Vector3				m_LocalPosition;
		Quaternion			m_LocalRotation;
		Vector3				m_LocalScale;

		mutable bool 		m_bDirty;
    };
}
