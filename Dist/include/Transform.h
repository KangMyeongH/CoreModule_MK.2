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
			: Component(owner), m_ParentID(-1),
			  m_WorldMatrix(),
			  m_LocalPosition(0.f, 0.f, 0.f),
			  m_LocalRotation(0.f, 0.f, 0.f, 1.f),
			  m_LocalScale(1.f, 1.f, 1.f),
			  m_bDirty(true)
		{
		}

		~Transform() override = default;
		Transform(const Transform& rhs);

    public:
		//======================================//
		//				 property				//
		//======================================//

		Vector3 GetLocalPosition() const { return m_LocalPosition; }
		void SetLocalPosition(const Vector3& position)
		{
			if (m_LocalPosition != position)
			{
				m_LocalPosition = position;
				setDirty();
			}
		}

		Quaternion GetLocalRotation() const { return m_LocalRotation; }
		void SetLocalRotation(const Quaternion& rotation)
		{
			if (m_LocalRotation != rotation)
			{
				m_LocalRotation = rotation;
				setDirty();
			}
		}

		Vector3 GetLocalScale() const { return m_LocalScale; }
		void SetLocalScale(const Vector3& scale)
		{
			if (m_LocalScale != scale)
			{
				m_LocalScale = scale;
				setDirty();
			}
		}

		Vector3 Position() const
		{
			updateMatrixIfNeeded();
			return m_WorldPosition;
		}

		void SetPosition(const Vector3& position)
		{
			if (const auto parent = m_Parent.lock())
			{
				const _matrix parentMat = parent->GetWorldMatrix();
				const _matrix inverseParentMat = XMMatrixInverse(nullptr, parentMat);
				const _vector worldPos = position.ToVector();
				const _vector localPos = XMVector3TransformCoord(worldPos, inverseParentMat);

				SetLocalPosition(Vector3::FromVector(localPos));
			}

			else
			{
				SetLocalPosition(position);
			}
		}

		Quaternion Rotation() const
		{
			updateMatrixIfNeeded();
			return m_WorldRotation;
		}

		void SetRotation(const Quaternion& rotation)
		{
			if (const auto parent = m_Parent.lock())
			{
				const Quaternion parentWorldRotation = parent->Rotation();
				const Quaternion invParentRotation = Quaternion::Inverse(parentWorldRotation);
				const Quaternion localRotation = invParentRotation * rotation;

				SetLocalRotation(localRotation);
			}

			else
			{
				SetLocalRotation(rotation);
			}
		}

		Vector3 Scale() const
		{
			updateMatrixIfNeeded();
			return m_WorldScale;
		}

		void SetScale(const Vector3& scale)
		{
			if (const auto parent = m_Parent.lock())
			{
				updateMatrixIfNeeded();

				m_WorldScale = scale;

				const _matrix matScale 	= DirectX::XMMatrixScalingFromVector(m_WorldScale.ToVector());
				const _matrix matRot 	= DirectX::XMMatrixRotationQuaternion(m_WorldRotation.ToVector());
				const _matrix matTrans 	= DirectX::XMMatrixTranslationFromVector(m_WorldPosition.ToVector());

				const _matrix matWorld 	= matScale * matRot * matTrans;

				const _matrix parentWorld = parent->GetWorldMatrix();

				_matrix localMat = XMMatrixMultiply(matWorld, XMMatrixInverse(nullptr, parentWorld));

				_vector localScale, localRot;
				XMMatrixDecompose(&localScale, &localRot, nullptr, localMat);

				SetLocalRotation(Quaternion::FromVector(localRot));
				SetLocalScale(Vector3::FromVector(localScale));
			}

			else
			{
				SetLocalScale(scale);
			}
		}

		_matrix GetWorldMatrix() const
		{
			updateMatrixIfNeeded();
			return XMLoadFloat4x4(&m_WorldMatrix);
		}

		SharedPtr<Transform> 	GetParent() const;
		void 					SetParent(const SharedPtr<Transform>& parent);

		std::vector<SharedPtr<Transform>>* GetChildren();

		Vector3 GetLocalEuler() const
		{
			return DecomposeEulerWithHistory(m_LocalRotation, m_LocalEulerAngles);
		}

		void SetLocalEuler(const Vector3 euler)
		{
			m_LocalEulerAngles = euler;
			SetLocalRotation(Quaternion::Euler(m_LocalEulerAngles));
		}

		_int					GetParentID() const;

		//======================================//
		//				  method				//
		//======================================//

		Vector3 Forward() const
		{
			updateMatrixIfNeeded();
			return Vector3(m_WorldMatrix._31, m_WorldMatrix._32, m_WorldMatrix._33).Normalized();
		}

		void Translate(const Vector3& value)
		{
			if (value != Vector3::Zero())
			{
				const Vector3 position = m_LocalPosition + value;
				SetLocalPosition(position);
			}
		}

		void Destroy() override;
		SharedPtr<Component> Clone() const override;


    private:
		void updateMatrixIfNeeded() const;

		void addChild(const SharedPtr<Transform>& child);

		void detachChild(const SharedPtr<Transform>& child);

		void setDirty();

		void registerComponent() override {}

		static SharedPtr<Transform> create();

    public:
		void to_json(nlohmann::ordered_json& j) override;
		void from_json(const nlohmann::ordered_json& j) override;
		friend void to_json(nlohmann::ordered_json& j, const SharedPtr<Transform>& t);
		friend void from_json(const nlohmann::ordered_json& j, const SharedPtr<Transform>& t);


	private:
		WeakPtr<Transform>					m_Parent;
		std::vector<SharedPtr<Transform>> 	m_Children;
		_int								m_ParentID;

		mutable _float4X4	m_WorldMatrix;

		mutable Vector3		m_WorldPosition;
		mutable Quaternion	m_WorldRotation;
		mutable Vector3		m_WorldScale;

		Vector3				m_LocalPosition;
		Quaternion			m_LocalRotation;
		Vector3				m_LocalScale;

		Vector3				m_LocalEulerAngles;

		mutable bool 		m_bDirty;
    };
}
