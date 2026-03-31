#ifndef CONTACT_MANIFOLD_HPP_DEFINED
#define CONTACT_MANIFOLD_HPP_DEFINED

#include "define.h"

// ─────────────────────────────────────────────────────────────────────────────
// ContactPoint
//
//  Un point de contact avec ses accumulateurs d'impulsion pour le warm start.
//  Les accumulateurs persistent d'une frame à l'autre via le ContactManifoldCache.
// ─────────────────────────────────────────────────────────────────────────────
struct ContactPoint
{
    XMFLOAT3 position       = { 0,0,0 };  // position monde du point de contact
    XMFLOAT3 localPointA    = { 0,0,0 };  // position locale sur le corps A (pour persistance)
    XMFLOAT3 localPointB    = { 0,0,0 };  // position locale sur le corps B

    // Accumulateurs warm start — persistés entre frames par ContactManifoldCache.
    float accumulatedNormalImpulse  = 0.0f;   // clampé à [0, +∞]
    float accumulatedTangentImpulse = 0.0f;   // clampé à [-μFn, +μFn] (cône de Coulomb)
    float accumulatedBinormalImpulse = 0.0f;  // second axe tangentiel
};


// ─────────────────────────────────────────────────────────────────────────────
// ContactManifold
//
//  Résultat de la narrow phase pour une paire (a, b).
//  Contient jusqu'à 4 points de contact (suffisant pour une face de box).
//
//  Convention normale : pointe de A vers B.
//  penetration > 0 signifie que les corps se chevauchent.
// ─────────────────────────────────────────────────────────────────────────────
struct ContactManifold
{
    EntityId a = 0;
    EntityId b = 0;

    XMFLOAT3 normal      = { 0,0,0 };  // A → B, normalisé
    float    penetration = 0.0f;

    static constexpr int kMaxPoints = 4;
    ContactPoint points[kMaxPoints];
    int          pointCount = 0;

    // Marqué true si le cache a transféré des accumulateurs de la frame précédente.
    bool isWarm = false;

    // Un des deux corps est-il un trigger ?
    bool isTrigger = false;
};


// ─────────────────────────────────────────────────────────────────────────────
// ContactManifoldCache
//
//  Persiste les manifolds entre frames pour le warm starting.
//
//  Cycle de vie :
//   [NarrowPhaseSystem::OnStartUpdate]
//     → BeginFrame() : sauvegarde les manifolds courants dans m_previous,
//                      vide m_manifolds.
//
//   [NarrowPhaseSystem::OnEndUpdate]
//     → AddManifold() : pour chaque nouveau manifold, cherche une entrée
//                       correspondante dans m_previous et transfère les
//                       accumulateurs (warm start).
//
//   [PhysicSystem::OnEndUpdate]
//     → Lit m_manifolds[], résout les contraintes, met à jour les accumulateurs.
// ─────────────────────────────────────────────────────────────────────────────
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
            _manifold.isWarm = true;

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

                if (bestIdx >= 0)
                {
                    cp.accumulatedNormalImpulse   = cached.points[bestIdx].accumulatedNormalImpulse;
                    cp.accumulatedTangentImpulse  = cached.points[bestIdx].accumulatedTangentImpulse;
                    cp.accumulatedBinormalImpulse = cached.points[bestIdx].accumulatedBinormalImpulse;
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
