#include "core_utility.h"

//void engine::core_utility::BuildSAH(uint32_t first, uint32_t count, std::vector<TriangleAABB>& tris,
//	std::vector<BVHNode>& nodes)
//{
//	BVHNode node{};
//	node.Box = tris[first].Box;
//	for (uint32_t i = 1; i < count; ++i)
//	{
//		node.Box = node.Box.Union(tris[first + i].Box);
//	}
//
//	if (count <= 4)
//	{
//		node.First = first;
//		node.Count = static_cast<uint16_t>(count);
//		node.Right = -1;
//		nodes.push_back(node);
//		return;
//	}
//
//	_float3 mean 	= { 0.f,0.f,0.f };
//	_float3 var 	= { 0.f, 0.f, 0.f };
//	for (uint32_t i = 0; i < count; ++i)
//	{
//		auto c = tris[first + i].Center;
//		mean.x += c.x;
//		mean.y += c.y;
//		mean.z += c.z;
//	}
//
//	_float invN = 1.f / static_cast<_float>(count);
//	mean.x *= invN;
//	mean.y *= invN;
//	mean.z *= invN;
//	for (uint32_t i = 0; i < count; ++i)
//	{
//		auto c = tris[first + i].Center;
//		var.x += (c.x - mean.x) * (c.x - mean.x);
//		var.y += (c.y - mean.y) * (c.y - mean.y);
//		var.z += (c.z - mean.z) * (c.z - mean.z);
//	}
//	int axis = (var.x > var.y && var.x > var.z) ? 0 : (var.y > var.z ? 1 : 2);
//
//	constexpr int BINS = 16;
//	struct Bin
//	{
//		AABB Box;
//		uint32_t Count = 0;
//	} bins[BINS];
//	_float minAxis = (&node.Box.Min.Value.x)[axis];
//	_float maxAxis = (&node.Box.Max.Value.x)[axis];
//	_float scale = maxAxis - minAxis > 0 ? BINS / (maxAxis - minAxis) : 0.f;
//
//	for (uint32_t i = 0; i < count; ++i)
//	{
//		_int binIdx = (std::min)(BINS - 1, static_cast<_int>(((&tris[first + i].Center.x)[axis] - minAxis) * scale));
//
//		Bin& bin = bins[binIdx];
//
//		if (bin.Count == 0)
//		{
//			bin.Box = tris[first + i].Box;
//		}
//
//		else
//		{
//			bin.Box = bin.Box.Union(tris[first + i].Box);
//		}
//
//		++bin.Count;
//	}
//
//	AABB leftBox[BINS - 1], rightBox[BINS - 1];
//	uint32_t leftCount[BINS - 1]{}, rightCount[BINS - 1]{};
//	AABB cur;
//	uint32_t cnt = 0;
//	for (_int i = 0; i < BINS - 1; ++i)
//	{
//		cnt += bins[i].Count;
//		cur = cnt == bins[i].Count ? bins[i].Box : cur.Union(bins[i].Box);
//		leftBox[i] = cur;
//		leftCount[i] = cnt;
//	}
//	cur = {}; cnt = 0;
//	for (int i = BINS - 1; i > 0; --i)
//	{
//		cnt += bins[i].Count;
//		cur = cnt == bins[i].Count ? bins[i].Box : cur.Union(bins[i].Box);
//		rightBox[i - 1] = cur;
//		rightCount[i - 1] = cnt;
//	}
//
//	_float parentArea = node.Box.SurfaceArea();
//	_float bestCost = kIntersectCost * static_cast<_float>(count);
//	_int   bestSplit = -1;
//	for (int i = 0; i < BINS - 1; ++i)
//	{
//		if (!leftCount[i] || !rightCount[i])
//		{
//			continue;
//		}
//
//		_float cost = kTraversalCost +
//			(leftBox[i].SurfaceArea() * static_cast<_float>(leftCount[i]) +
//				rightBox[i].SurfaceArea() * static_cast<_float>(rightCount[i])) / parentArea * kIntersectCost;
//
//		if (cost < bestCost)
//		{
//			bestCost = cost;
//			bestSplit = i;
//		}
//	}
//
//	if (bestSplit == -1)
//	{
//		node.First = first;
//		node.Count = static_cast<uint16_t>(count);
//		node.Right = -1;
//		nodes.push_back(node);
//		return;
//	}
//
//	uint32_t mid = std::partition(&tris[first], &tris[first + count],
//		[&](const TriangleAABB& t)
//		{
//			int binIdx = (std::min)(BINS - 1,
//				static_cast<int>(((&t.Center.x)[axis] - minAxis) * scale));
//			return binIdx <= bestSplit;
//		}) - &tris[0];
//
//	uint32_t nodeIdx = nodes.size();
//	nodes.push_back(node);               // 자리 확보
//	BuildSAH(first, mid - first, tris, nodes); // left
//	int rightOffset = nodes.size();
//	BuildSAH(mid, first + count - mid, tris, nodes); // right
//
//	nodes[nodeIdx].First = nodeIdx + 1;  // left child = next node
//	nodes[nodeIdx].Count = 0;            // interior
//	nodes[nodeIdx].Right = rightOffset;  // right child index
//}
void engine::MeshCollider::Build(const std::vector<DirectX::XMFLOAT3>& verts, const std::vector<uint32_t>& idx)
{
    const uint32_t triCnt = static_cast<uint32_t>(idx.size() / 3);
    mTris.resize(triCnt);
    mLocal = AABB();

    for (uint32_t i = 0; i < triCnt; ++i)
    {
        const DirectX::XMFLOAT3& v0 = verts[idx[i * 3 + 0]];
        const DirectX::XMFLOAT3& v1 = verts[idx[i * 3 + 1]];
        const DirectX::XMFLOAT3& v2 = verts[idx[i * 3 + 2]];
        AABB box(v0, v0);
        box.Expand(v1); box.Expand(v2);

        TriangleAABB t;
        t.Box = box;
        t.Center = { (v0.x + v1.x + v2.x) / 3.f,
                     (v0.y + v1.y + v2.y) / 3.f,
                     (v0.z + v1.z + v2.z) / 3.f };

        t.TriIndex = i;

        t.V0 = v0;
        t.V1 = v1;
        t.V2 = v2;

        mTris[i] = t;
        mLocal = (i == 0) ? box : mLocal.Union(box);
    }
    mNodes.clear();
    core_utility::BuildSAH(0, triCnt, mTris, mNodes);
}

bool engine::MeshCollider::Raycast(const Ray& ray, HitResult& outHit) const
{
    if (!core_utility::RayAABB(ray, mLocal, outHit.T, outHit.T)) return false;

    struct StackItem { uint32_t idx; float t; };
    std::stack<StackItem> st;
    st.push({ 0u, 0.f });

    bool found = false;
    while (!st.empty())
    {
        StackItem top = st.top();
        st.pop();
        uint32_t idx = top.idx;
        const BVHNode& n = mNodes[idx];
        float tmin, tmax;
        if (!core_utility::RayAABB(ray, n.Box, tmin, tmax) || (found && tmin >= outHit.T)) continue;

        if (n.Count)        // leaf
        {
            for (uint32_t i = 0; i < n.Count; ++i)
            {
                const TriangleAABB& t = mTris[n.First + i];
                float tHit;
                if (core_utility::RayTriangle(ray, t.V0, t.V1, t.V2, tHit) && tHit < outHit.T)
                {
                    outHit.T = tHit;
                    outHit.Tri = t.TriIndex;
                    found = true;
                }
            }
        }
        else               // internal
        {
            st.push({ idx + 1, tmin });
            st.push({ static_cast<uint32_t>(n.Right), tmin });
        }
    }
    return found;
}
