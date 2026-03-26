#ifndef PHYSIC_SYSTEM_H_DEFINED
#define PHYSIC_SYSTEM_H_DEFINED

#include "../ECS/ISystem.hpp"
#include "../Components/TransformComponent.hpp"
#include "../Components/ColliderComponent.hpp"
#include "../Components/PhysicComponent.hpp"
#include "../ContactManager.hpp"

class PhysicSystem : public System<PhysicComponent, TransformComponent>
{
public:
    void OnStartUpdate(float _dt) override;
    void OnUpdate(float _dt, EntityId _e, PhysicComponent& _physic, TransformComponent& _transform) override;
    void OnEndUpdate(float _dt) override;

    void SetContactManager(ContactManager* _contactManager) { m_pContactManager = _contactManager; }

private:
    struct ContactPointContext
    {
        XMFLOAT3 point;

        XMFLOAT3 centerA;
        XMFLOAT3 centerB;

        XMFLOAT3 rA;
        XMFLOAT3 rB;

        XMFLOAT3 velocityAtPointA;
        XMFLOAT3 velocityAtPointB;
        XMFLOAT3 relativeVelocity;
    };

private:
    void ResolveAllOverlaps();
    void ResolveAllImpulses(int _iterations = 4);

    void ResolveOverlap(PhysicComponent& _physicA, PhysicComponent& _physicB, Contact& _contact);
    void ResolveImpulseAtPoint(PhysicComponent& _physicA, PhysicComponent& _physicB,
        Contact& _contact, const XMFLOAT3& _point);

    ContactPointContext BuildContactPointContext(PhysicComponent& _physicA, PhysicComponent& _physicB, EntityId _entityA, EntityId _entityB, const XMFLOAT3& _point) const;

    float ComputeNormalImpulseScalar(PhysicComponent& _physicA, PhysicComponent& _physicB,
        const Contact& _contact, const ContactPointContext& _ctx, int _pointCount) const;

    float ComputeTangentImpulseScalar(PhysicComponent& _physicA, PhysicComponent& _physicB,
        const Contact& _contact, const ContactPointContext& _ctx, const XMFLOAT3& _tangent, int _pointCount) const;

    XMFLOAT3 ComputeTangent(const XMFLOAT3& _relativeVelocity, const XMFLOAT3& _normal) const;

    XMFLOAT3 ApplyInertiaInverse(const XMFLOAT3& _v, const XMFLOAT3& _inertiaInverse) const;
    float ComputeAngularEffectiveMassTerm(const XMFLOAT3& _r, const XMFLOAT3& _axis, const XMFLOAT3& _inertiaInverse) const;

    XMFLOAT3 GetCenter(EntityId _e) const;
    XMFLOAT3 GetVelocityAtPoint(const PhysicComponent& _physic, EntityId _e, const XMFLOAT3& _point) const;

    void WakeBodiesFromContact(PhysicComponent& _physicA, PhysicComponent& _physicB, Contact& _contact);
    void UpdateSupportContact(PhysicComponent& _physicA, PhysicComponent& _physicB, Contact& _contact);
private:
    ContactManager* m_pContactManager = nullptr;
};

#endif // !PHYSIC_SYSTEM_H_DEFINED