#pragma once
#include <chrono>
#include <random>
#include <stack>

#include "core_defines.h"

constexpr float kTraversalCost = 1.f;
constexpr float kIntersectCost = 1.f; 

namespace engine
{
    //class COREMODULE_API MeshCollider
    //{
    //public:
    //    // vertices : 로컬 공간 정점, indices : 삼각형 당 3개
    //    void        Build(const std::vector<DirectX::XMFLOAT3>& verts,
    //        const std::vector<uint32_t>& idx);

    //    bool        Raycast(const Ray& ray, HitResult& outHit) const;
    //    const std::vector<BVHNode>& BVH()   const noexcept { return mNodes; }
    //    const std::vector<TriangleAABB>& Tris()  const noexcept { return mTris; }

    //    // 로컬 AABB (전체)
    //    const AABB& LocalBounds() const noexcept { return mLocal; }

    //private:
    //    std::vector<TriangleAABB> mTris;
    //    std::vector<BVHNode>      mNodes;
    //    AABB                      mLocal;
    //};

    class COREMODULE_API core_utility
    {
    //public:
    //    static void BuildSAH(uint32_t first, uint32_t count, std::vector<TriangleAABB>& tris, std::vector<BVHNode>& nodes)
    //    {
    //        constexpr int   BINS = 16;   
    //        constexpr float Ct = 1.0f;   
    //        constexpr float Ci = 2.0f;   
    //        constexpr uint32_t kLeaf = 4;


    //        BVHNode node{};
    //        node.Box = tris[first].Box;
    //        for (uint32_t i = 1; i < count; ++i)
    //            node.Box = node.Box.Union(tris[first + i].Box);

    //        if (count <= kLeaf)
    //        {
    //            node.First = first;
    //            node.Count = static_cast<uint16_t>(count);
    //            node.Right = -1;
    //            nodes.push_back(node);
    //            return;
    //        }

    //        int   bestAxis = -1;
    //        int   bestBinIdx = -1;
    //        float bestCost = Ci * count;                 // leaf 비용
    //        float bestInvLen = 0.f;
    //        float bestMinA = 0.f;                        // 축 최소값 저장

    //        struct Bin { AABB box; uint32_t cnt = 0; };
    //        std::array<Bin, BINS> bins{};                  // 재사용할 버퍼

    //        auto Surface = [](const AABB& b)
    //            { return b.SurfaceArea(); };

    //        const float rootArea = node.Box.SurfaceArea();

    //        for (int axis = 0; axis < 3; ++axis)
    //        {
    //            float lenAxis = (&node.Box.Max.Value.x)[axis] - (&node.Box.Min.Value.x)[axis];
    //            if (lenAxis < 1e-5f) continue;

    //            // 빈 버퍼 초기화
    //            for (int i = 0; i < BINS; ++i)
    //            {
    //                bins[i].cnt = 0;
    //                bins[i].box = AABB();
    //            }

    //            float invLen = 1.0f / lenAxis;
    //            float minAxis = (&node.Box.Min.Value.x)[axis];

    //            for (uint32_t i = 0; i < count; ++i)
    //            {
    //                float c = (&tris[first + i].Center.x)[axis];
    //                int   idx = int(((c - minAxis) * invLen) * BINS);
    //                idx = (std::min)(BINS - 1, (std::max)(0, idx));

    //                Bin& b = bins[idx];
    //                b.cnt++;
    //                b.box = (b.cnt == 1) ? tris[first + i].Box
    //                    : b.box.Union(tris[first + i].Box);
    //            }

    //            std::array<AABB, BINS>     lbox{}, rbox{};
    //            std::array<uint32_t, BINS> lcnt{}, rcnt{};

    //            AABB     acc;  uint32_t cc = 0;
    //            for (int i = 0; i < BINS; ++i)            // prefix
    //            {
    //                if (bins[i].cnt)
    //                {
    //                    acc = (cc == 0) ? bins[i].box : acc.Union(bins[i].box);
    //                    cc += bins[i].cnt;
    //                }
    //                lbox[i] = acc; lcnt[i] = cc;
    //            }
    //            acc = AABB(); cc = 0;
    //            for (int i = BINS - 1; i >= 0; --i)       // suffix
    //            {
    //                if (bins[i].cnt)
    //                {
    //                    acc = (cc == 0) ? bins[i].box : acc.Union(bins[i].box);
    //                    cc += bins[i].cnt;
    //                }
    //                rbox[i] = acc; rcnt[i] = cc;
    //            }

    //            for (int i = 0; i < BINS - 1; ++i)
    //            {
    //                if (lcnt[i] == 0 || rcnt[i + 1] == 0) continue;

    //                float cost = Ct
    //                    + (lcnt[i] * Surface(lbox[i])
    //                        + rcnt[i + 1] * Surface(rbox[i + 1])) / rootArea * Ci;

    //                if (cost < bestCost)
    //                {
    //                    bestCost = cost;
    //                    bestAxis = axis;
    //                    bestBinIdx = i;
    //                    bestInvLen = invLen;
    //                    bestMinA = minAxis;
    //                }
    //            }
    //        }

    //        if (bestAxis == -1)
    //        {
    //            node.First = first;
    //            node.Count = static_cast<uint16_t>(count);
    //            node.Right = -1;
    //            nodes.push_back(node);
    //            return;
    //        }

    //        auto midIter = std::partition(
    //            tris.begin() + first, tris.begin() + first + count,
    //            [&](const TriangleAABB& t)
    //            {
    //                float c = (&t.Center.x)[bestAxis];
    //                int   idx = int(((c - bestMinA) * bestInvLen) * BINS);
    //                idx = (std::min)(BINS - 1, (std::max)(0, idx));
    //                return idx <= bestBinIdx;
    //            });

    //        uint32_t leftCount = uint32_t(midIter - (tris.begin() + first));
    //        uint32_t rightFirst = first + leftCount;

    //        if (leftCount == 0 || leftCount == count)
    //        {
    //            node.First = first;
    //            node.Count = static_cast<uint16_t>(count);
    //            node.Right = -1;
    //            nodes.push_back(node);
    //            return;
    //        }

    //        uint32_t parentIdx = static_cast<uint32_t>(nodes.size());
    //        nodes.push_back(node);                      // placeholder

    //        // LEFT
    //        BuildSAH(first, leftCount, tris, nodes);

    //        // RIGHT
    //        uint32_t rightRootIdx = static_cast<uint32_t>(nodes.size());
    //        BuildSAH(rightFirst, count - leftCount, tris, nodes);

    //        // 부모 노드 완성
    //        nodes[parentIdx] = node;
    //        nodes[parentIdx].First = first;
    //        nodes[parentIdx].Count = 0;
    //        nodes[parentIdx].Right = static_cast<int32_t>(rightRootIdx);
    //    }

    //    static void BuildCentroid(uint32_t first, uint32_t count,
    //        std::vector<TriangleAABB>& tris,
    //        std::vector<BVHNode>& nodes)
    //    {
    //        constexpr uint32_t kLeafLimit = 4;

    //        // 1. 노드 및 전체 AABB
    //        BVHNode node{};
    //        node.Box = tris[first].Box;
    //        for (uint32_t i = 1; i < count; ++i)
    //            node.Box = node.Box.Union(tris[first + i].Box);

    //        // 2. Leaf 조건
    //        if (count <= kLeafLimit)
    //        {
    //            node.First = first;
    //            node.Count = static_cast<uint16_t>(count);
    //            node.Right = -1;
    //            nodes.push_back(node);
    //            return;
    //        }

    //        // 3. 가장 긴 축 선택
    //        const float len[3] = { node.Box.Max.Value.x - node.Box.Min.Value.x,
    //                               node.Box.Max.Value.y - node.Box.Min.Value.y,
    //                               node.Box.Max.Value.z - node.Box.Min.Value.z };
    //        int axis = 0;
    //        if (len[1] > len[axis]) axis = 1;
    //        if (len[2] > len[axis]) axis = 2;


    //        std::vector<float> centers(count);
    //        for (uint32_t i = 0; i < count; ++i)
    //            centers[i] = (&tris[first + i].Center.x)[axis];
    //        std::nth_element(centers.begin(),
    //            centers.begin() + count / 2,
    //            centers.end());
    //        float splitPos = centers[count / 2];

  
    //        auto midIter = std::partition(
    //            tris.begin() + first, tris.begin() + first + count,
    //            [&](const TriangleAABB& t)
    //            {
    //                return (&t.Center.x)[axis] < splitPos;
    //            });
    //        uint32_t leftCount = uint32_t(midIter - (tris.begin() + first));
    //        uint32_t rightFirst = first + leftCount;


    //        if (leftCount == 0 || leftCount == count)
    //        {
    //            node.First = first;
    //            node.Count = static_cast<uint16_t>(count);
    //            node.Right = -1;
    //            nodes.push_back(node);
    //            return;
    //        }

    //        uint32_t parentIdx = static_cast<uint32_t>(nodes.size());
    //        nodes.push_back(node);

    //        BuildCentroid(first, leftCount, tris, nodes);
    //        uint32_t rightRootIdx = static_cast<uint32_t>(nodes.size());
    //        BuildCentroid(rightFirst, count - leftCount, tris, nodes);

    //        nodes[parentIdx] = node;
    //        nodes[parentIdx].First = first;
    //        nodes[parentIdx].Count = 0;
    //        nodes[parentIdx].Right = static_cast<int32_t>(rightRootIdx);
    //    }

    //    static _bool Validate(uint32_t idx,
    //                          const std::vector<BVHNode>& nodes,
    //                          const std::vector<TriangleAABB>& tris,
    //                          std::vector<uint8_t>& triVisited)
    //    {
    //        const BVHNode& n = nodes[idx];

    //        if (n.Count == 0)   // internal
    //        {
    //            const BVHNode& left = nodes[idx + 1];
    //            const BVHNode& right = nodes[n.Right];

    //            auto inside = [](const AABB& p, const AABB& c)
    //                {
    //                    return (p.Min.Value.x <= c.Min.Value.x && p.Max.Value.x >= c.Max.Value.x) &&
    //                        (p.Min.Value.y <= c.Min.Value.y && p.Max.Value.y >= c.Max.Value.y) &&
    //                        (p.Min.Value.z <= c.Min.Value.z && p.Max.Value.z >= c.Max.Value.z);
    //                };
    //            if (!inside(n.Box, left.Box)) { LOG_FAIL("parent ⊃ left 실패");  return false; }
    //            if (!inside(n.Box, right.Box)) { LOG_FAIL("parent ⊃ right 실패"); return false; }

    //            return Validate(idx + 1, nodes, tris, triVisited) &&
    //                Validate(n.Right, nodes, tris, triVisited);
    //        }
    //        // leaf
    //        for (uint32_t i = 0; i < n.Count; ++i)
    //        {
    //            uint32_t tIdx = tris[n.First + i].TriIndex;
    //            if (triVisited[tIdx])
    //            {
    //                LOG_FAIL("삼각형 중복 " << tIdx); return false;
    //            }
    //            triVisited[tIdx] = 1;
    //        }
    //        return true;
    //    }

    //    static _bool ValidateBVH(const std::vector<BVHNode>& nodes,
    //        const std::vector<TriangleAABB>& tris)
    //    {
    //        if (nodes.empty())
    //        {
    //            return true;
    //        }
    //        std::vector<uint8_t> visited(tris.size(), 0);
    //        bool ok = Validate(0, nodes, tris, visited);

    //        for (uint8_t v : visited) ok = ok && (v == 1);
    //        return ok;
    //    }

    //    static void DumpAABBsToOBJ(const std::vector<BVHNode>& nodes,
    //        const char* path,
    //        int maxDepth = 3)
    //    {
    //        FILE* fp = nullptr;
    //        if (fopen_s(&fp, path, "w") != 0 || !fp)
    //            return;
    //        auto corner = [](const AABB& b, int i) -> DirectX::XMFLOAT3 {
    //            return {
    //                (i & 1) ? b.Max.Value.x : b.Min.Value.x,
    //                (i & 2) ? b.Max.Value.y : b.Min.Value.y,
    //                (i & 4) ? b.Max.Value.z : b.Min.Value.z };
    //            };
    //        uint32_t vbase = 1;
    //        struct Stack { uint32_t idx; int depth; };
    //        std::stack<Stack> st; st.push({ 0, 0 });
    //        while (!st.empty())
    //        {
    //            Stack top = st.top();
    //            st.pop();
    //            uint32_t idx = top.idx;
    //            int      d = top.depth;
    //            const BVHNode& n = nodes[idx];
    //            if (d > maxDepth) continue;

    //            for (int i = 0; i < 8; ++i)
    //            {
    //                auto p = corner(n.Box, i);
    //                fprintf(fp, "v %f %f %f\n", p.x, p.y, p.z);
    //            }
    //            static const int idxs[12][2] = {
    //                {0,1},{0,2},{0,4},{1,3},{1,5},{2,3},{2,6},{3,7},
    //                {4,5},{4,6},{5,7},{6,7} };
    //            for (auto& e : idxs)
    //                fprintf(fp, "l %u %u\n", vbase + e[0], vbase + e[1]);
    //            vbase += 8;

    //            if (n.Count == 0)
    //            {
    //                st.push({ idx + 1, d + 1 });
    //                st.push({ static_cast<uint32_t>(n.Right), d + 1 });
    //            }
    //        }
    //        fclose(fp);
    //    }

    //    template<typename BuildFn, typename RaycastFn>
    //    static void CompareRaycastPerformance(BuildFn buildBVH,
    //        RaycastFn  bvhRay,
    //        const std::vector<DirectX::XMFLOAT3>& verts,
    //        const std::vector<uint32_t>& indices)
    //    {
    //        using clk = std::chrono::high_resolution_clock;

    //        std::mt19937 rng(42);
    //        std::uniform_real_distribution<float> dist(-10.f, 10.f);

    //        auto shootRay = [&](const Ray& ray, HitResult& outHit)
    //            {

    //                outHit.T = std::numeric_limits<float>::max();
    //                bool hit = false;
    //                for (size_t i = 0; i < indices.size(); i += 3)
    //                {
    //                    const DirectX::XMFLOAT3& v0 = verts[indices[i]];
    //                    const DirectX::XMFLOAT3& v1 = verts[indices[i + 1]];
    //                    const DirectX::XMFLOAT3& v2 = verts[indices[i + 2]];
    //                    float t;
    //                    if (RayTriangle(ray, v0, v1, v2, t) && t < outHit.T)
    //                    {
    //                        outHit.T = t; hit = true;
    //                    }
    //                }
    //                return hit;
    //            };


    //        MeshCollider col; col.Build(verts, indices);

    //        const int N = 10000;
    //        int mismatch = 0;
    //        auto t0 = clk::now();
    //        for (int i = 0; i < N; ++i)
    //        {
    //            Ray r; r.Origin = DirectX::XMVectorSet(dist(rng), dist(rng), dist(rng), 0);
    //            r.Direction = DirectX::XMVector3Normalize(DirectX::XMVectorSet(dist(rng), dist(rng), dist(rng), 0));

    //            HitResult brute, accel;
    //            bool h0 = shootRay(r, brute);
    //            bool h1 = col.Raycast(r, accel);

    //            if (h0 != h1 || (h0 && fabs(brute.T - accel.T) > 1e-3f))
    //                ++mismatch;
    //        }
    //        auto t1 = clk::now();
    //        auto bruteMS = std::chrono::duration<double, std::milli>(t1 - t0).count();

    //        t0 = clk::now();
    //        for (int i = 0; i < N; ++i)
    //        {
    //            Ray r; r.Origin = DirectX::XMVectorSet(dist(rng), dist(rng), dist(rng), 0);
    //            r.Direction = DirectX::XMVector3Normalize(DirectX::XMVectorSet(dist(rng), dist(rng), dist(rng), 0));
    //            HitResult accel;
    //            col.Raycast(r, accel);
    //        }
    //        t1 = clk::now();
    //        auto bvhMS = std::chrono::duration<double, std::milli>(t1 - t0).count();

    //        std::cout << "Ray compare: mismatch " << mismatch << " / " << N << '\n';
    //        std::cout << "Brute ms = " << bruteMS << ",  BVH ms = " << bvhMS
    //            << "   (speed-up x" << bruteMS / bvhMS << ")\n";
    //    }

    //    static _bool RayAABB(const Ray& ray, const AABB& b, _float& tmin, _float& tmax)
    //    {
    //        const DirectX::XMFLOAT3 o = DirectX::XMFLOAT3(DirectX::XMVectorGetX(ray.Origin.ToVector()),
    //                                                      DirectX::XMVectorGetY(ray.Origin.ToVector()),
    //                                                      DirectX::XMVectorGetZ(ray.Origin.ToVector()));
    //        const DirectX::XMFLOAT3 d = DirectX::XMFLOAT3(DirectX::XMVectorGetX(ray.Direction.ToVector()),
    //                                                      DirectX::XMVectorGetY(ray.Direction.ToVector()),
    //                                                      DirectX::XMVectorGetZ(ray.Direction.ToVector()));
    //        tmin = 0.f;  tmax = std::numeric_limits<float>::max();
    //        for (int i = 0; i < 3; ++i)
    //        {
    //            float invD = 1.f / (&d.x)[i];
    //            float t1 = ((&b.Min.Value.x)[i] - (&o.x)[i]) * invD;
    //            float t2 = ((&b.Max.Value.x)[i] - (&o.x)[i]) * invD;
    //            if (invD < 0.f) std::swap(t1, t2);
    //            tmin = (t1 > tmin) ? t1 : tmin;
    //            tmax = (t2 < tmax) ? t2 : tmax;
    //            if (tmin > tmax) return false;
    //        }
    //        return true;
    //    }

    //    static bool RayTriangle(const Ray& ray,
    //        const DirectX::XMFLOAT3& v0,
    //        const DirectX::XMFLOAT3& v1,
    //        const DirectX::XMFLOAT3& v2,
    //        float& tHit)
    //    {
    //        const DirectX::XMVECTOR O = ray.Origin.ToVector();
    //        const DirectX::XMVECTOR D = ray.Direction.ToVector();
    //        const DirectX::XMVECTOR V0 = DirectX::XMLoadFloat3(&v0);
    //        const DirectX::XMVECTOR V1 = DirectX::XMLoadFloat3(&v1);
    //        const DirectX::XMVECTOR V2 = DirectX::XMLoadFloat3(&v2);

    //        const DirectX::XMVECTOR e1 = DirectX::XMVectorSubtract(V1, V0);
    //        const DirectX::XMVECTOR e2 = DirectX::XMVectorSubtract(V2, V0);
    //        const DirectX::XMVECTOR p = DirectX::XMVector3Cross(D, e2);
    //        float det = DirectX::XMVectorGetX(DirectX::XMVector3Dot(e1, p));
    //        if (fabs(det) < 1e-6f) return false;
    //        const float invDet = 1.0f / det;
    //        const DirectX::XMVECTOR s = DirectX::XMVectorSubtract(O, V0);
    //        float u = DirectX::XMVectorGetX(DirectX::XMVector3Dot(s, p)) * invDet;
    //        if (u < 0.f || u>1.f) return false;
    //        const DirectX::XMVECTOR q = DirectX::XMVector3Cross(s, e1);
    //        float v = DirectX::XMVectorGetX(DirectX::XMVector3Dot(D, q)) * invDet;
    //        if (v < 0.f || u + v>1.f) return false;
    //        tHit = DirectX::XMVectorGetX(DirectX::XMVector3Dot(e2, q)) * invDet;
    //        return tHit > 0.f;
    //    }

    //    static _float ComputeSAHCost(const std::vector<BVHNode>& nodes, const AABB& root)
    //    {
    //        float cost = 0.f;
    //        std::stack<uint32_t> st; st.push(0);
    //        while (!st.empty())
    //        {
    //            uint32_t idx = st.top(); st.pop();
    //            const BVHNode& n = nodes[idx];
    //            float areaRatio = n.Box.SurfaceArea() / root.SurfaceArea();
    //            if (n.Count)
    //                cost += areaRatio * n.Count;      // Ci = 1
    //            else
    //            {
    //                cost += areaRatio;                // Ct = 1
    //                st.push(idx + 1);
    //                st.push(n.Right);
    //            }
    //        }
    //        return cost;
    //    }
    public:
        static _float Random(const _float min, const _float max)
        {
            _float value = rand() / static_cast<_float>(RAND_MAX);
            return min + (max - min) * value;
        }
    };
}
