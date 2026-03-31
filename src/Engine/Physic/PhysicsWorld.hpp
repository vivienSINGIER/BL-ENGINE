#pragma once
// ============================================================
//  PhysicsWorld.hpp
//  Orchestrateur central, équivalent au IPhysicsEnvironment
//  du Source Engine (vphysics.dll).
//
//  Usage :
//    PhysicsWorld world;
//    world.SetGravity({0,-9.81f,0});
//
//    RigidBody* box = world.CreateBody();
//    box->MakeBox(1.0f, {0.5f,0.5f,0.5f});
//    box->position = {0,5,0};
//    box->useGravity = true;
//
//    // Chaque frame :
//    world.Step(dt);
//
//    // Sync vers ECS :
//    world.ForEachBody([&](RigidBody& rb) {
//        ecs.GetComponent<Transform>(rb.id).SetPosition(rb.position);
//    });
// ============================================================
#include "RigidBody.hpp"
#include "Contact.hpp"
#include "BroadPhase.hpp"
#include "NarrowPhase.hpp"
#include "ConstraintSolver.hpp"
#include "Integrator.hpp"
#include <vector>
#include <memory>
#include <unordered_map>
#include <functional>
#include <algorithm>
#include <unordered_set>
#include <cassert>

// Signature du callback trigger (optionnel)
using TriggerCallback = std::function<void(uint32_t idA, uint32_t idB, bool enter)>;

class PhysicsWorld
{
public:
    // ---- Configuration ----
    void SetGravity(const XMFLOAT3& g)     { m_integrator.gravity = g; }
    void SetSolverIterations(int n)          { m_solver.iterations = n; }
    void SetPositionCorrectionFactor(float f){ m_solver.baumgarteFactor = f; }
    void SetRestitutionThreshold(float v)    { m_solver.restitutionThreshold = v; }
    void SetTriggerCallback(TriggerCallback cb) { m_triggerCb = cb; }

    // ---- Gestion des corps ----
    RigidBody* CreateBody(uint32_t entityId)
    {
        assert(m_bodies.find(entityId) == m_bodies.end() && "EntityId already registered");
        auto rb       = std::make_unique<RigidBody>();
        rb->id        = entityId;
        RigidBody* ptr = rb.get();
        m_bodies[entityId] = std::move(rb);
        // Insertion BVH après première mise à jour (AABB dépend de la position)
        m_pendingInsert.push_back(entityId);
        return ptr;
    }

    void DestroyBody(uint32_t entityId)
    {
        auto bvhIt = m_bvhHandles.find(entityId);
        if (bvhIt != m_bvhHandles.end())
        {
            m_broadPhase.Remove(bvhIt->second);
            m_bvhHandles.erase(bvhIt);
        }
        m_bodies.erase(entityId);
    }

    RigidBody* GetBody(uint32_t entityId)
    {
        auto it = m_bodies.find(entityId);
        return it != m_bodies.end() ? it->second.get() : nullptr;
    }

    // ---- Sync depuis ECS (avant Step) ----
    //  Appeler si le Transform a été modifié côté ECS (teleport, animation)
    void SyncBodyFromECS(uint32_t entityId, const XMFLOAT3& pos, const XMFLOAT4& rot)
    {
        RigidBody* rb = GetBody(entityId);
        if (!rb) return;
        rb->position    = pos;
        rb->orientation = rot;
    }

    // ---- Itération ----
    void ForEachBody(const std::function<void(RigidBody&)>& fn)
    {
        for (auto& [id, rb] : m_bodies) fn(*rb);
    }

    // ---- Step principal ----
    void Step(float dt)
    {
        // 0) Insère les nouveaux corps dans le BVH
        _FlushPendingInserts();

        // 1) Intégration des forces → nouvelles vitesses et positions tentatives
        for (auto& [id, rb] : m_bodies)
            m_integrator.Integrate(*rb, dt);

        // 2) Mise à jour BVH (seulement les corps qui ont bougé)
        _UpdateBVH();

        // 3) Broad phase → paires candidates
        std::vector<CandidatePair> pairs;
        m_broadPhase.CollectCandidatePairs(pairs);
        _DeduplicatePairs(pairs);

        // 4) Narrow phase → contacts
        m_contacts.clear();
        for (auto& [a, b] : pairs)
        {
            RigidBody* rbA = GetBody(a);
            RigidBody* rbB = GetBody(b);
            if (!rbA || !rbB) continue;
            if (rbA->bodyType == BodyType::Static && rbB->bodyType == BodyType::Static) continue;

            auto contact = m_narrowPhase.Test(*rbA, *rbB);
            if (!contact) continue;

            if (contact->isTrigger)
            {
                _HandleTrigger(a, b, true);
                continue;
            }

            // Support contact (pour sleep sur sol)
            _UpdateSupportContact(*rbA, *rbB, *contact);

            // Réveille les corps si impact significatif
            _WakeBodiesOnContact(*rbA, *rbB, *contact);

            m_contacts.push_back(*contact);
        }

        // 5) Résolution des contraintes (PGS + warm starting)
        m_solver.getBody = [this](uint32_t id) -> RigidBody* { return GetBody(id); };
        m_solver.Solve(m_contacts, m_contactCache, dt);

        // 6) Correction positionnelle (split impulse)
        m_solver.SolvePositions(m_contacts);

        // 7) Mise à jour du tenseur d'inertie pour tous les corps dynamiques
        for (auto& [id, rb] : m_bodies)
            if (rb->bodyType == BodyType::Dynamic && !rb->isSleeping && rb->allowRotation)
                rb->inertia.UpdateWorldTensor(rb->orientation);
    }

    // ---- Raycasting (stub – style Source Engine) ----
    struct RayHit
    {
        uint32_t  entityId = ~0u;
        XMFLOAT3  point    = {};
        XMFLOAT3  normal   = {};
        float     t        = FLT_MAX;
    };
    bool Raycast(const XMFLOAT3& origin, const XMFLOAT3& dir, float maxDist, RayHit& hit) const
    {
        bool found = false;
        hit.t = maxDist;
        for (const auto& [id, rb] : m_bodies)
        {
            float t;
            XMFLOAT3 n;
            if (_RayVsBody(*rb, origin, dir, maxDist, t, n) && t < hit.t)
            {
                hit.entityId = id;
                hit.point    = V3Add(origin, V3Scale(dir, t));
                hit.normal   = n;
                hit.t        = t;
                found        = true;
            }
        }
        return found;
    }

    // ---- Debug ----
    int  BodyCount()   const { return (int)m_bodies.size(); }
    int  ContactCount() const { return (int)m_contacts.size(); }
    const std::vector<Contact>& GetContacts() const { return m_contacts; }

private:
    std::unordered_map<uint32_t, std::unique_ptr<RigidBody>> m_bodies;
    std::unordered_map<uint32_t, int>  m_bvhHandles; // entityId → BVH node idx
    std::vector<uint32_t>              m_pendingInsert;

    DynamicBVH       m_broadPhase;
    NarrowPhase      m_narrowPhase;
    ConstraintSolver m_solver;
    Integrator       m_integrator;
    ContactCache     m_contactCache;

    std::vector<Contact>            m_contacts;
    std::unordered_set<uint64_t>    m_activeTriggers; // paires trigger actives
    TriggerCallback                 m_triggerCb;

    // ---- Helpers ----

    void _FlushPendingInserts()
    {
        for (uint32_t id : m_pendingInsert)
        {
            RigidBody* rb = GetBody(id);
            if (!rb) continue;
            AABB3 aabb = _ComputeAABB(*rb);
            int handle = m_broadPhase.Insert(id, aabb);
            m_bvhHandles[id] = handle;
        }
        m_pendingInsert.clear();
    }

    void _UpdateBVH()
    {
        for (auto& [id, rb] : m_bodies)
        {
            if (rb->isSleeping || rb->bodyType == BodyType::Static) continue;
            auto it = m_bvhHandles.find(id);
            if (it == m_bvhHandles.end()) continue;
            AABB3 newAABB = _ComputeAABB(*rb);
            m_broadPhase.Update(it->second, newAABB);
        }
    }

    AABB3 _ComputeAABB(const RigidBody& rb) const
    {
        XMFLOAT3 c = rb.GetColliderCenter();
        AABB3 aabb;
        if (rb.shape == ColliderShape::Sphere)
        {
            aabb.min = V3Sub(c, {rb.radius, rb.radius, rb.radius});
            aabb.max = V3Add(c, {rb.radius, rb.radius, rb.radius});
        }
        else if (rb.shape == ColliderShape::Box)
        {
            // OBB → AABB (projection des demi-extents sur chaque axe world)
            XMMATRIX rot = XMMatrixRotationQuaternion(XMLoadFloat4(&rb.orientation));
            XMFLOAT3 axes[3];
            XMStoreFloat3(&axes[0], XMVector3TransformNormal(XMVectorSet(1,0,0,0), rot));
            XMStoreFloat3(&axes[1], XMVector3TransformNormal(XMVectorSet(0,1,0,0), rot));
            XMStoreFloat3(&axes[2], XMVector3TransformNormal(XMVectorSet(0,0,1,0), rot));
            float wx = fabsf(axes[0].x)*rb.halfExtents.x + fabsf(axes[1].x)*rb.halfExtents.y + fabsf(axes[2].x)*rb.halfExtents.z;
            float wy = fabsf(axes[0].y)*rb.halfExtents.x + fabsf(axes[1].y)*rb.halfExtents.y + fabsf(axes[2].y)*rb.halfExtents.z;
            float wz = fabsf(axes[0].z)*rb.halfExtents.x + fabsf(axes[1].z)*rb.halfExtents.y + fabsf(axes[2].z)*rb.halfExtents.z;
            aabb.min = { c.x-wx, c.y-wy, c.z-wz };
            aabb.max = { c.x+wx, c.y+wy, c.z+wz };
        }
        else // Capsule
        {
            float r = rb.radius + rb.capsuleHalfHeight;
            aabb.min = V3Sub(c, {r,r,r});
            aabb.max = V3Add(c, {r,r,r});
        }
        return aabb;
    }

    void _DeduplicatePairs(std::vector<CandidatePair>& pairs)
    {
        std::sort(pairs.begin(), pairs.end());
        pairs.erase(std::unique(pairs.begin(), pairs.end()), pairs.end());
    }

    void _UpdateSupportContact(RigidBody& rbA, RigidBody& rbB, const Contact& c)
    {
        // Normal pointant vers le bas sur A → A est supporté par B
        if (c.normal.y < -0.5f)
        {
            rbA.hasSupportContact = true;
            rbA.supportNormal     = V3Neg(c.normal);
        }
        // Normal pointant vers le haut sur B → B est supporté par A
        if (c.normal.y > 0.5f)
        {
            rbB.hasSupportContact = true;
            rbB.supportNormal     = c.normal;
        }
    }

    void _WakeBodiesOnContact(RigidBody& rbA, RigidBody& rbB, const Contact& c)
    {
        XMFLOAT3 relV = V3Sub(rbB.linearVelocity, rbA.linearVelocity);
        float vn = V3Dot(relV, c.normal);
        if (vn < -0.3f)
        {
            rbA.WakeUp();
            rbB.WakeUp();
        }
    }

    void _HandleTrigger(uint32_t a, uint32_t b, bool active)
    {
        uint64_t key = ((uint64_t)std::min(a,b) << 32) | std::max(a,b);
        if (active)
        {
            bool isNew = m_activeTriggers.insert(key).second;
            if (isNew && m_triggerCb) m_triggerCb(a, b, true);
        }
        else
        {
            bool removed = m_activeTriggers.erase(key) > 0;
            if (removed && m_triggerCb) m_triggerCb(a, b, false);
        }
    }

    // ---- Raycasting interne ----
    bool _RayVsBody(const RigidBody& rb, const XMFLOAT3& orig, const XMFLOAT3& dir,
                     float maxDist, float& tOut, XMFLOAT3& nOut) const
    {
        if (rb.shape == ColliderShape::Sphere)
            return _RayVsSphere(rb.GetColliderCenter(), rb.radius, orig, dir, maxDist, tOut, nOut);
        return _RayVsAABB(_ComputeAABB(rb), orig, dir, maxDist, tOut, nOut);
    }

    bool _RayVsSphere(const XMFLOAT3& c, float r, const XMFLOAT3& orig, const XMFLOAT3& dir,
                       float maxDist, float& tOut, XMFLOAT3& nOut) const
    {
        XMFLOAT3 oc = V3Sub(orig, c);
        float a = V3Dot(dir, dir);
        float b = 2*V3Dot(oc, dir);
        float disc = b*b - 4*a*(V3Dot(oc,oc)-r*r);
        if (disc < 0) return false;
        float t = (-b - sqrtf(disc)) / (2*a);
        if (t < 0 || t > maxDist) return false;
        tOut = t;
        nOut = V3Normalize(V3Sub(V3Add(orig, V3Scale(dir,t)), c));
        return true;
    }

    bool _RayVsAABB(const AABB3& box, const XMFLOAT3& orig, const XMFLOAT3& dir,
                     float maxDist, float& tOut, XMFLOAT3& nOut) const
    {
        float tmin = 0, tmax = maxDist;
        int axis = -1;
        float dirArr[3]  = {dir.x, dir.y, dir.z};
        float origArr[3] = {orig.x, orig.y, orig.z};
        float minArr[3]  = {box.min.x, box.min.y, box.min.z};
        float maxArr[3]  = {box.max.x, box.max.y, box.max.z};

        for (int i = 0; i < 3; ++i)
        {
            if (fabsf(dirArr[i]) < 1e-8f)
            {
                if (origArr[i] < minArr[i] || origArr[i] > maxArr[i]) return false;
            }
            else
            {
                float t1 = (minArr[i]-origArr[i]) / dirArr[i];
                float t2 = (maxArr[i]-origArr[i]) / dirArr[i];
                if (t1 > t2) std::swap(t1,t2);
                if (t1 > tmin) { tmin = t1; axis = i; }
                tmax = Minf(tmax, t2);
                if (tmin > tmax) return false;
            }
        }
        if (axis < 0) return false;
        tOut = tmin;
        nOut = {};
        float* nArr = &nOut.x;
        nArr[axis] = dir.x*nArr[0]+dir.y*nArr[1]+dir.z*nArr[2] < 0 ? 1.0f : -1.0f;
        return true;
    }
};

// ============================================================
//  Implémentation de ComputeAABB nécessitée par BroadPhase.hpp
//  (définie ici car dépend de RigidBody)
// ============================================================
inline AABB3 ComputeAABB(const RigidBody& rb)
{
    PhysicsWorld tmp; // ne crée pas de corps
    // Utilise la même logique
    XMFLOAT3 c = rb.GetColliderCenter();
    AABB3 aabb;
    if (rb.shape == ColliderShape::Sphere)
    {
        aabb.min = V3Sub(c, {rb.radius, rb.radius, rb.radius});
        aabb.max = V3Add(c, {rb.radius, rb.radius, rb.radius});
    }
    else
    {
        XMMATRIX rot = XMMatrixRotationQuaternion(XMLoadFloat4(&rb.orientation));
        XMFLOAT3 axes[3];
        XMStoreFloat3(&axes[0], XMVector3TransformNormal(XMVectorSet(1,0,0,0), rot));
        XMStoreFloat3(&axes[1], XMVector3TransformNormal(XMVectorSet(0,1,0,0), rot));
        XMStoreFloat3(&axes[2], XMVector3TransformNormal(XMVectorSet(0,0,1,0), rot));
        float wx = fabsf(axes[0].x)*rb.halfExtents.x + fabsf(axes[1].x)*rb.halfExtents.y + fabsf(axes[2].x)*rb.halfExtents.z;
        float wy = fabsf(axes[0].y)*rb.halfExtents.x + fabsf(axes[1].y)*rb.halfExtents.y + fabsf(axes[2].y)*rb.halfExtents.z;
        float wz = fabsf(axes[0].z)*rb.halfExtents.x + fabsf(axes[1].z)*rb.halfExtents.y + fabsf(axes[2].z)*rb.halfExtents.z;
        aabb.min = { c.x-wx, c.y-wy, c.z-wz };
        aabb.max = { c.x+wx, c.y+wy, c.z+wz };
    }
    return aabb;
}
