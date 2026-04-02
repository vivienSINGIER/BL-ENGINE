#ifndef CONTACT_MANIFOLD_HPP_DEFINED
#define CONTACT_MANIFOLD_HPP_DEFINED

#include "define.h"

struct ContactPoint
{
    XMFLOAT3 position       = { 0,0,0 };  // position monde du point de contact
    XMFLOAT3 localPointA    = { 0,0,0 };  // position locale sur le corps A (pour persistance)
    XMFLOAT3 localPointB    = { 0,0,0 };  // position locale sur le corps B
};

struct ContactManifold
{
    EntityId a = 0;
    EntityId b = 0;

    XMFLOAT3 normal      = { 0,0,0 };  // A → B, normalisé
    float    penetration = 0.0f;

    static constexpr int kMaxPoints = 4;
    ContactPoint points[kMaxPoints];
    int          pointCount = 0;

    // Un des deux corps est-il un trigger ?
    bool isTrigger = false;
};

struct ContactManifoldCache
{
    Vector<ContactManifold> manifolds;

    void BeginFrame()
    {
        m_previous.clear();
        for (const ContactManifold& m : manifolds)
            m_previous[MakeKey(m.a, m.b)] = m;
        manifolds.clear();
    }

    void AddManifold(ContactManifold& _manifold)
    {
        uint64 key = MakeKey(_manifold.a, _manifold.b);
        auto it = m_previous.find(key);

        if (it != m_previous.end())
        {
            const ContactManifold& cached = it->second;

            // Transfert des accumulateurs par appariement spatial.
            // On apparie par proximité des points locaux sur A (stable entre frames).
            for (int i = 0; i < _manifold.pointCount; ++i)
            {
                ContactPoint& cp = _manifold.points[i];
                float bestDistSq = kMatchThresholdSq;
                int   bestIdx    = -1;

                for (int j = 0; j < cached.pointCount; ++j)
                {
                    const ContactPoint& prev = cached.points[j];
                    XMFLOAT3 diff =
                    {
                        cp.localPointA.x - prev.localPointA.x,
                        cp.localPointA.y - prev.localPointA.y,
                        cp.localPointA.z - prev.localPointA.z
                    };
                    float distSq = diff.x*diff.x + diff.y*diff.y + diff.z*diff.z;

                    if (distSq < bestDistSq)
                    {
                        bestDistSq = distSq;
                        bestIdx    = j;
                    }
                }
            }
        }

        manifolds.push_back(_manifold);
    }

private:
    static constexpr float kMatchThreshold   = 0.02f;
    static constexpr float kMatchThresholdSq = kMatchThreshold * kMatchThreshold;

    std::unordered_map<uint64, ContactManifold> m_previous;

    inline uint64 MakeKey(EntityId _a, EntityId _b) const
    {
        if (_a > _b) std::swap(_a, _b);
        return (static_cast<uint64>(static_cast<uint32>(_a)) << 32) |
                static_cast<uint32>(_b);
    }
};

#endif // !CONTACT_MANIFOLD_HPP_DEFINED
