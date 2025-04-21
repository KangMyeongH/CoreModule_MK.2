#pragma once
#include <typeindex>

#include "Object.h"
#include "ScriptBehaviourManager.h"
#include "Transform.h"

namespace engine
{
	class Collision;

	namespace editor
	{
		class EditorComponentManager;
		class Hierarchy;
	}

	class Component;

	using Components = std::unordered_map<std::type_index, std::vector<SharedPtr<Component>>>;

	class COREMODULE_API GameObject : public Object
	{
		friend class Scene;
		friend class editor::Hierarchy;
		friend class editor::EditorComponentManager;
		friend class CollisionManager;

	protected:
		//======================================//
		//				constructor				//
		//======================================//

		GameObject(const _string& name = "GameObject");
		~GameObject() override;

		GameObject(const GameObject& rhs)
			: Object(rhs),
			m_AssetPath(rhs.m_AssetPath),
			m_bActiveSelf(rhs.m_bActiveSelf),
			m_bActiveInHierarchy(rhs.m_bActiveInHierarchy),
			m_bStatic(rhs.m_bStatic)
		{
			m_Transform = std::static_pointer_cast<Transform>(rhs.m_Transform->Clone());
			m_Transform->SetOwner(nullptr);

			for (const auto& pair : rhs.m_Components)
			{
				const auto& type = pair.first;
				const auto& components = pair.second;
				std::vector<std::shared_ptr<Component>> clonedComponents;
				clonedComponents.reserve(components.size());

				for (const auto& component : components)
				{
					auto clonedComponent = component->Clone();
					clonedComponent->SetOwner(nullptr);
					clonedComponents.push_back(std::move(clonedComponent));
				}

				m_Components.emplace(type, std::move(clonedComponents));
			}
		}

	public:
		//======================================//
		//				  method				//
		//======================================//

		template <typename T, typename... Args>
		SharedPtr<T> AddComponent(Args&&... args)
		{
			static_assert(std::is_base_of<Component, T>::value, "T must be derived from Component");

			const _string typeName = StripMsvcClassName(typeid(T).name());

			SharedPtr<Component> component = ComponentFactory::GetInstance().CreateComponent(typeName);

			if (component)
			{
				component->SetOwner(std::static_pointer_cast<GameObject>(shared_from_this()));
				m_Components[typeid(T)].push_back(component);
				component->registerComponent();
			}

			return std::static_pointer_cast<T>(component);
		}

		template <typename T>
		SharedPtr<T> GetComponent()
		{
			static_assert(std::is_base_of<Component, T>::value, "T must be derived from Component");

			auto it = m_Components.find(typeid(T));
			if (it != m_Components.end() && !it->second.empty())
			{
				return std::static_pointer_cast<T>(it->second.front());
			}

			return nullptr;
		}

		Components& GetComponents() { return m_Components; }

		void RemoveComponent(const SharedPtr<Component>& component)
		{
			for (auto it = m_Components.begin(); it != m_Components.end();)
			{
				if (!it->second.empty() && it->second.front() == component)
				{
					m_Components.erase(it);
					return;
				}

				++it;
			}
		}

		SharedPtr<Transform> 			GetTransform() const { return m_Transform; }

		void 							SetActive(bool active);
		_bool 							IsActive() const;

		_bool							IsActiveSelf() const { return m_bActiveSelf; }
		_bool 							IsActiveInHierarchy() const { return m_bActiveInHierarchy; }

		void 							SetTag(const _string& tag);
		_string 						GetTag() const { return m_Tag; }

		void							SetAssetPath(const _wstring& path) { m_AssetPath = path; }
		_wstring						GetAssetPath() const { return m_AssetPath; }

		_bool							IsStatic() const { return m_bStatic; }

		SharedPtr<GameObject>			FindGameObject(const std::string& name) const;

		static SharedPtr<GameObject> 	Create(const _string& name = "GameObject", ApplicationMode mode = CLIENT);
		SharedPtr<GameObject> 			Clone(ApplicationMode mode = CLIENT) const;
		void 							Destroy() override;

	private:
		void updateActiveHierarchy()
		{
			auto parent = m_Transform->GetParent();

			bool newActive = m_bActiveSelf && (parent ? parent->GetGameObject().lock()->IsActiveInHierarchy() : true);

			if (m_bActiveInHierarchy == newActive)
			{
				return;
			}

			m_bActiveInHierarchy = newActive;

			for (const auto& child : *m_Transform->GetChildren())
			{
				child->GetGameObject().lock()->updateActiveHierarchy();
			}
		}


		void onCollisionEnter(const Collision& other);
		void onCollisionStay(const Collision& other);
		void onCollisionExit(const Collision& other);
		//======================================//
		//				 serialize				//
		//======================================//
	public:
		static void ToJson(nlohmann::ordered_json& j, const SharedPtr<GameObject>& obj, ApplicationMode mode);
		static void FromJson(const nlohmann::ordered_json& j, const SharedPtr<GameObject>& obj, ApplicationMode mode);

	private:
		//======================================//
		//				  fields				//
		//======================================//

		Components						m_Components;
		SharedPtr<Transform>			m_Transform;
		_string							m_Tag;
		_wstring						m_AssetPath;
		_bool							m_bActiveSelf;
		_bool							m_bActiveInHierarchy;
		_bool							m_bStatic;
	};
}
