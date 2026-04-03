#ifndef PHYSIC_SYSTEM_H_DEFINED
#define PHYSIC_SYSTEM_H_DEFINED

#include "../ECS/ISystem.h"
#include "../Components/RigidBodyComponent.hpp"
#include "../Components/MotionComponent.hpp"
#include "../Components/TransformComponent.hpp"
#include "NarrowPhaseSystem.h"
#include "../Core/Utils.hpp"

class PhysicSystem : public System<>
{
public:
    void Update(float _dt) override;
    void SetNarrowPhaseSystem(NarrowPhaseSystem* _np) { m_narrowPhase = _np; }

private:
    void ResolvePenetrations();
    void ResolveVelocities(float _dt);

    XMFLOAT3 VelocityAtPoint(MotionComponent& _motion, const XMFLOAT3& _r) const;
    XMFLOAT3 ApplyInertiaInverse(const XMFLOAT3& _v, const float _t[9]) const;
    float AngularMassTerm(const XMFLOAT3& _r, const XMFLOAT3& _axis, const float _t[9]) const;

    XMFLOAT3 Snap(const XMFLOAT3& _v, float _threshold) const;

    MotionComponent&    GetMotion(EntityId _e);
    RigidBodyComponent* GetRigid(EntityId _e);
    XMFLOAT3            GetCenter(EntityId _e) const;

private:
    NarrowPhaseSystem* m_narrowPhase = nullptr;
    MotionComponent    m_nullMotion;   // fallback pour entités sans MotionComponent

    static constexpr float kLinearSnapThreshold = 0.1f;
    static constexpr float kAngularSnapThreshold = 0.1f;

    static constexpr float kRestitutionThreshold = 0.3f;  // en dessous : restitution = 0

	static constexpr float kSleepThreshold = 0.5f;

    static constexpr float kPenetrationSlop = 0.01f;
    static constexpr float kBeta = 0.5f;
};

#endif // !PHYSIC_SYSTEM_H_DEFINED