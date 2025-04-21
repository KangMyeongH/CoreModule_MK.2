#pragma once
#include "core_defines.h"

namespace engine
{
	class GameObject;
	class Collider;
}

namespace engine
{
    struct CollisionData
    {
        CollisionData() : A(nullptr), B(nullptr), Penetration(0)
        {
	        
        }

        CollisionData(const SharedPtr<Collider>& a, const SharedPtr<Collider>& b, const Vector3& normal, const _float penetration)
	        : A(a), B(b), Normal(normal), Penetration(penetration)
		{

		}

        CollisionData(const CollisionData& rhs)
	        : A(rhs.A), B(rhs.B), Normal(rhs.Normal), Penetration(rhs.Penetration)
        {
	        
        }

        // Ãæµ¹ ½Ö
        SharedPtr<Collider> A;
        SharedPtr<Collider> B;

        // ÃÖ¼Ò °ãÄ§ Ãà
        Vector3 Normal;

        // °ãÄ§·®
        _float Penetration;
    };

    class Collision
    {
        //======================================//
        //				constructor				//
        //======================================//
    public:
        Collision(const SharedPtr<GameObject>& other, const SharedPtr<Collider>& collider)
	        : m_Other(other), m_Collider(collider), m_Penetration(0)
        {
        }

        //======================================//
        //				 property				//
        //======================================//
    public:
    	SharedPtr<GameObject> GetGameObject() const { return m_Other; }
        SharedPtr<Collider> GetCollider() const { return m_Collider; }

        //======================================//
        //				  method				//
        //======================================//

        //======================================//
        //				  fields				//
        //======================================//
    private:
        SharedPtr<GameObject> 	m_Other;
        SharedPtr<Collider> 	m_Collider;
        Vector3 				m_Normal;
        _float 					m_Penetration;
    };
}
