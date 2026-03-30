#pragma once
// ============================================================
//  Contact.hpp
//  Structure de contact + cache pour le warm starting
//  (style Bullet/VPhysics : les impulses de la frame précédente
//   sont réinjectées au début du solver).
// ============================================================
#include "PhysicsMath.hpp"
#include <array>
#include <cstdint>
#include <unordered_map>

constexpr int MAX_CONTACT_POINTS = 4;

struct ContactPoint
{
    XMFLOAT3 position      = {0,0,0};  // world space
    float    penetration   = 0.0f;

    // Impulses accumulées (warm starting)
    float    normalImpulse  = 0.0f;
    float    tangentImpulse[2] = {0,0};

    // Bras de levier (mis en cache pour éviter recalcul dans le solver)
    XMFLOAT3 rA = {0,0,0};
    XMFLOAT3 rB = {0,0,0};

    // Masse effective pré-calculée
    float normalEffMass   = 0.0f;
    float tangentEffMass[2] = {0,0};

    // Biais de position (Baumgarte ou position correction)
    float positionBias = 0.0f;
};

struct Contact
{
    uint32_t idA = 0, idB = 0;

    XMFLOAT3  normal      = {0,1,0};  // de B vers A
    XMFLOAT3  tangent[2]  = {};        // plan de friction (orthonormal)

    int  pointCount = 0;
    std::array<ContactPoint, MAX_CONTACT_POINTS> points;

    bool isTrigger = false;

    // Construit les deux tangentes de Gram-Schmidt
    void BuildTangentFrame()
    {
        XMFLOAT3 n = normal;
        XMFLOAT3 t;
        if (fabsf(n.x) < 0.57f)
            t = V3Normalize(V3Cross(n, {1,0,0}));
        else
            t = V3Normalize(V3Cross(n, {0,1,0}));
        tangent[0] = t;
        tangent[1] = V3Cross(n, t);
    }
};

// ============================================================
//  ContactKey : identifie une paire de corps de manière unique
// ============================================================
struct ContactKey
{
    uint64_t value;
    ContactKey(uint32_t a, uint32_t b)
    {
        uint32_t lo = a < b ? a : b;
        uint32_t hi = a < b ? b : a;
        value = (uint64_t)lo << 32 | hi;
    }
    bool operator==(const ContactKey& o) const { return value == o.value; }
};
struct ContactKeyHash
{
    size_t operator()(const ContactKey& k) const { return std::hash<uint64_t>{}(k.value); }
};

// ============================================================
//  ContactCache  –  warm starting
//  Garde les impulses de la frame N pour les pré-charger en N+1
// ============================================================
struct CachedImpulses
{
    float normalImpulse     = 0;
    float tangentImpulse[2] = {0,0};
};

class ContactCache
{
public:
    void Store(const ContactKey& key, int pointIdx, const ContactPoint& cp)
    {
        auto& vec = m_cache[key];
        if (pointIdx >= (int)vec.size()) vec.resize(pointIdx+1);
        vec[pointIdx].normalImpulse    = cp.normalImpulse;
        vec[pointIdx].tangentImpulse[0] = cp.tangentImpulse[0];
        vec[pointIdx].tangentImpulse[1] = cp.tangentImpulse[1];
    }

    bool Retrieve(const ContactKey& key, int pointIdx, CachedImpulses& out) const
    {
        auto it = m_cache.find(key);
        if (it == m_cache.end()) return false;
        if (pointIdx >= (int)it->second.size()) return false;
        out = it->second[pointIdx];
        return true;
    }

    void Clear() { m_cache.clear(); }

private:
    std::unordered_map<ContactKey, std::vector<CachedImpulses>, ContactKeyHash> m_cache;
};
