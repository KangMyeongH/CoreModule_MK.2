#pragma once
#include "core_defines.h"
#include "Collision.h"

namespace engine
{
	class Collider;
    
    struct ColliderPairHash
    {
	    size_t operator()(const std::pair<SharedPtr<Collider>, SharedPtr<Collider>>& val) const
	    {
            auto pRaw1 = val.first.get();
            auto pRaw2 = val.second.get();

            // 주소 기준 정렬
            auto p1 = (pRaw1 < pRaw2) ? pRaw1 : pRaw2;
            auto p2 = (pRaw1 < pRaw2) ? pRaw2 : pRaw1;

            // 주소 기준 해시
            return std::hash<void*>()(p1) ^ std::hash<void*>()(p2);
	    }
    };

    struct ColliderPairEq
    {
	    _bool operator()(const std::pair<SharedPtr<Collider>, SharedPtr<Collider>>& a, 
            const std::pair<SharedPtr<Collider>, SharedPtr<Collider>>& b) const
	    {
            auto a1 = a.first.get();
            auto a2 = a.second.get();

            auto b1 = b.first.get();
            auto b2 = b.second.get();

            a1 = (a1 < a2) ? a1 : a2;
            a2 = (a1 < a2) ? a2 : a1;

            b1 = (b1 < b2) ? b1 : b2;
            b2 = (b1 < b2) ? b2 : b1;

            return a1 == b1 && a2 == b2;
	    }
    };

	class COREMODULE_API CollisionManager
    {
        //======================================//
        //				constructor				//
        //======================================//
    private:
        CollisionManager();
        ~CollisionManager();
    public:
        DECLARE_SINGLETON(CollisionManager)

        //======================================//
        //				 property				//
        //======================================//
    public:
        void AddCollider(const SharedPtr<Collider>& collider);

        //======================================//
        //				  method				//
        //======================================//
    public:

        void ColliderUpdate();

        void RenderCollider(const ComPtr<ID3D11DeviceContext>& context, const _float4X4& viewMat, const _float4X4& projMat);

        void RegisterCollider();
        void FlushDestroyCollider();

        void Release();

	private:
        //std::unique_ptr<BVHNode> buildBVHTopDown(std::vector<SharedPtr<Collider>>& colliders, _int depth = 0);

        void broadPhaseSap(std::vector<std::pair<SharedPtr<Collider>, SharedPtr<Collider>>>& outPotentialPairs);

        void narrowPhase(const std::vector<std::pair<SharedPtr<Collider>, SharedPtr<Collider>>>& potentialPairs);

        void processCollisionResults(const std::unordered_map<std::pair<SharedPtr<Collider>, SharedPtr<Collider>>, CollisionData, ColliderPairHash, ColliderPairEq>& newCollisionMap);

        void invokeCollisionEnter(const SharedPtr<Collider>& a, const SharedPtr<Collider>& b, const Vector3& normal, const _float& penetration);
        void invokeCollisionStay(const SharedPtr<Collider>& a, const SharedPtr<Collider>& b, const Vector3& normal, const _float& penetration);
        void invokeCollisionExit(const SharedPtr<Collider>& a, const SharedPtr<Collider>& b, const Vector3& normal, const _float& penetration);



        _bool checkCollider(const SharedPtr<Collider>& a, const SharedPtr<Collider>& b, Vector3& outNormal, _float& outPenetration);

        // Box, Sphere, Capsule, Mesh
        _bool checkBoxBox(const SharedPtr<Collider>& a, const SharedPtr<Collider>& b, Vector3& outNormal, _float& outPenetration);
        _bool checkBoxCapsule(const SharedPtr<Collider>& a, const SharedPtr<Collider>& b, Vector3& outNormal, _float& outPenetration);
        _bool checkBoxMesh(const SharedPtr<Collider>& a, const SharedPtr<Collider>& b, Vector3& outNormal, _float& outPenetration);
        _bool checkBoxSphere(const SharedPtr<Collider>& a, const SharedPtr<Collider>& b, Vector3& outNormal, _float& outPenetration);
        _bool checkCapsuleCapsule(const SharedPtr<Collider>& a, const SharedPtr<Collider>& b, Vector3& outNormal, _float& outPenetration);
        _bool checkCapsuleMesh(const SharedPtr<Collider>& a, const SharedPtr<Collider>& b, Vector3& outNormal, _float& outPenetration);
        _bool checkCapsuleSphere(const SharedPtr<Collider>& a, const SharedPtr<Collider>& b, Vector3& outNormal, _float& outPenetration);
        _bool checkMeshMesh(const SharedPtr<Collider>& a, const SharedPtr<Collider>& b, Vector3& outNormal, _float& outPenetration);
        _bool checkMeshSphere(const SharedPtr<Collider>& a, const SharedPtr<Collider>& b, Vector3& outNormal, _float& outPenetration);
        _bool checkSphereSphere(const SharedPtr<Collider>& a, const SharedPtr<Collider>& b, Vector3& outNormal, _float& outPenetration);


        //======================================//
        //				  fields				//
        //======================================//
        std::vector<SharedPtr<Collider>> 	m_Colliders;
        std::vector<SharedPtr<Collider>>    m_DynamicColliders;

        std::unordered_map<std::pair<SharedPtr<Collider>, SharedPtr<Collider>>, CollisionData, ColliderPairHash, ColliderPairEq> m_CollisionMap;
        std::list<SharedPtr<Collider>> 		m_RegisterQueue;
    };
}
