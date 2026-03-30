#pragma once
// ============================================================
//  BroadPhase.hpp
//  AABB Tree 3D dynamique (Dynamic BVH)
//  Remplace la grille 2D par un arbre hiérarchique 3D.
//  Style : Bullet dbvt / Source Engine spatial partition.
//
//  Algorithme :
//    - Chaque leaf = un RigidBody
//    - AABB gonflée d'un "fat factor" pour éviter rebuild continu
//    - Insertion O(log n) par best-sibling search
//    - Query O(log n + k) pairs candidates
// ============================================================
#include "PhysicsMath.hpp"
#include <vector>
#include <functional>
#include <cstdint>
#include <limits>

struct AABB3
{
    XMFLOAT3 min = { FLT_MAX, FLT_MAX, FLT_MAX };
    XMFLOAT3 max = {-FLT_MAX,-FLT_MAX,-FLT_MAX };

    float SurfaceArea() const
    {
        XMFLOAT3 d = V3Sub(max, min);
        return 2.0f*(d.x*d.y + d.y*d.z + d.z*d.x);
    }

    bool Overlaps(const AABB3& o) const
    {
        return min.x <= o.max.x && max.x >= o.min.x
            && min.y <= o.max.y && max.y >= o.min.y
            && min.z <= o.max.z && max.z >= o.min.z;
    }

    static AABB3 Union(const AABB3& a, const AABB3& b)
    {
        return {
            { Minf(a.min.x,b.min.x), Minf(a.min.y,b.min.y), Minf(a.min.z,b.min.z) },
            { Maxf(a.max.x,b.max.x), Maxf(a.max.y,b.max.y), Maxf(a.max.z,b.max.z) }
        };
    }

    XMFLOAT3 Center() const { return V3Scale(V3Add(min,max), 0.5f); }

    void Fatten(float margin)
    {
        min = V3Sub(min, {margin,margin,margin});
        max = V3Add(max, {margin,margin,margin});
    }

    bool Contains(const AABB3& o) const
    {
        return o.min.x >= min.x && o.min.y >= min.y && o.min.z >= min.z
            && o.max.x <= max.x && o.max.y <= max.y && o.max.z <= max.z;
    }
};

// ---- Calcul AABB depuis RigidBody (forward) ----
struct RigidBody; // forward

AABB3 ComputeAABB(const RigidBody& rb);

// ============================================================
constexpr int NULL_NODE = -1;
constexpr float FAT_MARGIN = 0.2f;  // marge de fattening (en unités monde)

struct BVHNode
{
    AABB3    aabb;
    int      parent   = NULL_NODE;
    int      children[2] = {NULL_NODE, NULL_NODE};
    uint32_t entityId = ~0u;       // valide seulement si leaf
    bool     isLeaf   = false;
    int      height   = 0;
};

using CandidatePair = std::pair<uint32_t,uint32_t>;

class DynamicBVH
{
public:
    DynamicBVH() { m_nodes.reserve(256); }

    // ---- Insertion / Mise à jour / Suppression ----
    int  Insert(uint32_t entityId, const AABB3& aabb);
    void Remove(int nodeIdx);
    bool Update(int nodeIdx, const AABB3& newAABB);  // retourne true si rebuild

    // ---- Query ----
    void CollectCandidatePairs(std::vector<CandidatePair>& out) const;

    // ---- Debug ----
    int NodeCount() const { return (int)m_nodes.size(); }

private:
    int  _AllocNode();
    void _FreeNode(int idx);
    int  _FindBestSibling(const AABB3& leafAABB) const;
    void _FixAncestors(int startNode);
    void _InsertLeaf(int leafIdx);
    void _RemoveLeaf(int leafIdx);
    void _QueryPairs(int nodeA, int nodeB, std::vector<CandidatePair>& out) const;

    std::vector<BVHNode> m_nodes;
    std::vector<int>     m_freeList;
    int                  m_root = NULL_NODE;
};

// ============================================================
//  Implémentation inline (header-only pour simplicité)
// ============================================================
inline int DynamicBVH::_AllocNode()
{
    if (!m_freeList.empty())
    {
        int idx = m_freeList.back();
        m_freeList.pop_back();
        m_nodes[idx] = BVHNode{};
        return idx;
    }
    m_nodes.emplace_back();
    return (int)m_nodes.size()-1;
}

inline void DynamicBVH::_FreeNode(int idx)
{
    m_nodes[idx].isLeaf = false;
    m_nodes[idx].entityId = ~0u;
    m_freeList.push_back(idx);
}

inline int DynamicBVH::Insert(uint32_t entityId, const AABB3& aabb)
{
    int leaf = _AllocNode();
    m_nodes[leaf].aabb = aabb;
    m_nodes[leaf].aabb.Fatten(FAT_MARGIN);
    m_nodes[leaf].entityId = entityId;
    m_nodes[leaf].isLeaf   = true;
    m_nodes[leaf].height   = 0;
    _InsertLeaf(leaf);
    return leaf;
}

inline void DynamicBVH::_InsertLeaf(int leafIdx)
{
    if (m_root == NULL_NODE) { m_root = leafIdx; m_nodes[leafIdx].parent = NULL_NODE; return; }

    int best = _FindBestSibling(m_nodes[leafIdx].aabb);

    // Crée un nœud interne
    int oldParent = m_nodes[best].parent;
    int newInternal = _AllocNode();
    m_nodes[newInternal].parent = oldParent;
    m_nodes[newInternal].aabb   = AABB3::Union(m_nodes[leafIdx].aabb, m_nodes[best].aabb);
    m_nodes[newInternal].isLeaf = false;

    if (oldParent != NULL_NODE)
    {
        if (m_nodes[oldParent].children[0] == best)
            m_nodes[oldParent].children[0] = newInternal;
        else
            m_nodes[oldParent].children[1] = newInternal;
    }
    else m_root = newInternal;

    m_nodes[newInternal].children[0] = best;
    m_nodes[newInternal].children[1] = leafIdx;
    m_nodes[best].parent    = newInternal;
    m_nodes[leafIdx].parent = newInternal;

    _FixAncestors(newInternal);
}

inline int DynamicBVH::_FindBestSibling(const AABB3& leafAABB) const
{
    // Greedy best-first search sur la surface area heuristic
    int best = m_root;
    float bestCost = AABB3::Union(m_nodes[m_root].aabb, leafAABB).SurfaceArea();
    float leafSA = leafAABB.SurfaceArea();

    std::vector<std::pair<int,float>> stack;
    stack.push_back({m_root, 0.0f});

    while (!stack.empty())
    {
        auto [node, inheritedCost] = stack.back(); stack.pop_back();

        AABB3 combined = AABB3::Union(m_nodes[node].aabb, leafAABB);
        float directCost = combined.SurfaceArea();
        float totalCost  = directCost + inheritedCost;

        if (totalCost < bestCost) { bestCost = totalCost; best = node; }

        if (!m_nodes[node].isLeaf)
        {
            float childInherited = inheritedCost + directCost - m_nodes[node].aabb.SurfaceArea();
            float lowerBound = leafSA + childInherited;
            if (lowerBound < bestCost)
            {
                stack.push_back({m_nodes[node].children[0], childInherited});
                stack.push_back({m_nodes[node].children[1], childInherited});
            }
        }
    }
    return best;
}

inline void DynamicBVH::_FixAncestors(int node)
{
    int curr = node;
    while (curr != NULL_NODE)
    {
        int c0 = m_nodes[curr].children[0];
        int c1 = m_nodes[curr].children[1];
        if (c0 != NULL_NODE && c1 != NULL_NODE)
        {
            m_nodes[curr].aabb   = AABB3::Union(m_nodes[c0].aabb, m_nodes[c1].aabb);
            m_nodes[curr].height = 1 + std::max(m_nodes[c0].height, m_nodes[c1].height);
        }
        curr = m_nodes[curr].parent;
    }
}

inline void DynamicBVH::Remove(int nodeIdx)
{
    _RemoveLeaf(nodeIdx);
    _FreeNode(nodeIdx);
}

inline void DynamicBVH::_RemoveLeaf(int leafIdx)
{
    if (leafIdx == m_root) { m_root = NULL_NODE; return; }

    int parent  = m_nodes[leafIdx].parent;
    int grandpa = m_nodes[parent].parent;
    int sibling = (m_nodes[parent].children[0] == leafIdx)
                   ? m_nodes[parent].children[1]
                   : m_nodes[parent].children[0];

    if (grandpa != NULL_NODE)
    {
        if (m_nodes[grandpa].children[0] == parent)
            m_nodes[grandpa].children[0] = sibling;
        else
            m_nodes[grandpa].children[1] = sibling;
        m_nodes[sibling].parent = grandpa;
        _FreeNode(parent);
        _FixAncestors(grandpa);
    }
    else
    {
        m_root = sibling;
        m_nodes[sibling].parent = NULL_NODE;
        _FreeNode(parent);
    }
}

inline bool DynamicBVH::Update(int nodeIdx, const AABB3& newAABB)
{
    // Si la fat AABB contient encore la nouvelle AABB, pas de rebuild
    if (m_nodes[nodeIdx].aabb.Contains(newAABB)) return false;

    _RemoveLeaf(nodeIdx);
    m_nodes[nodeIdx].aabb = newAABB;
    m_nodes[nodeIdx].aabb.Fatten(FAT_MARGIN);
    _InsertLeaf(nodeIdx);
    return true;
}

inline void DynamicBVH::_QueryPairs(int nodeA, int nodeB, std::vector<CandidatePair>& out) const
{
    if (nodeA == NULL_NODE || nodeB == NULL_NODE) return;
    if (!m_nodes[nodeA].aabb.Overlaps(m_nodes[nodeB].aabb)) return;

    if (m_nodes[nodeA].isLeaf && m_nodes[nodeB].isLeaf)
    {
        uint32_t a = m_nodes[nodeA].entityId;
        uint32_t b = m_nodes[nodeB].entityId;
        if (a != b) out.push_back({std::min(a,b), std::max(a,b)});
        return;
    }
    if (m_nodes[nodeA].isLeaf)
    {
        _QueryPairs(nodeA, m_nodes[nodeB].children[0], out);
        _QueryPairs(nodeA, m_nodes[nodeB].children[1], out);
    }
    else
    {
        _QueryPairs(m_nodes[nodeA].children[0], nodeB, out);
        _QueryPairs(m_nodes[nodeA].children[1], nodeB, out);
    }
}

inline void DynamicBVH::CollectCandidatePairs(std::vector<CandidatePair>& out) const
{
    if (m_root == NULL_NODE) return;
    // Self-collision query sur la racine
    if (!m_nodes[m_root].isLeaf)
    {
        _QueryPairs(m_nodes[m_root].children[0], m_nodes[m_root].children[1], out);
        // Récursion interne
        std::function<void(int)> recurse = [&](int node)
        {
            if (node == NULL_NODE || m_nodes[node].isLeaf) return;
            int c0 = m_nodes[node].children[0];
            int c1 = m_nodes[node].children[1];
            _QueryPairs(c0, c1, out);
            recurse(c0);
            recurse(c1);
        };
        recurse(m_root);
    }
    // Déduplique (utilisation d'un set si nécessaire pour très grand nombre)
}
