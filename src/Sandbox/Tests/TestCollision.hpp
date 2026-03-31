#ifndef TEST_COLLISION_HPP_DEFINED
#define TEST_COLLISION_HPP_DEFINED

#include "Test.h"
#include "../Engine/Engine.h"

class TestCollision : public Test
{
public:
    struct CameraScript : public IScript
    {
        void Update(float dt) override
        {
            TransformComponent& t = GetComponent<TransformComponent>();

            if (InputManager::IsKey(Z))
                t.local.Move(XMFLOAT3(0.0f, 0.0f, 2.0f * dt));
            if (InputManager::IsKey(S))
                t.local.Move(XMFLOAT3(0.0f, 0.0f, -2.0f * dt));
            if (InputManager::IsKey(Q))
                t.local.Move(XMFLOAT3(-2.0f * dt, 0.0f, 0.0f));
            if (InputManager::IsKey(D))
                t.local.Move(XMFLOAT3(2.0f * dt, 0.0f, 0.0f));
            if (InputManager::IsKey(SPACE))
                t.local.Move(XMFLOAT3(0.0f, 2.0f * dt, 0.0f));
            if (InputManager::IsKey(LCONTROL))
                t.local.Move(XMFLOAT3(0.0f, -2.0f * dt, 0.0f));
        }
    };

    // ─── Corps dynamique (cube) ───────────────────────────────────────────────────
    static EntityId CreateDynamicBox(World* world,
        const XMFLOAT3& position,
        const XMFLOAT3& halfExtents,
		const XMFLOAT3& initialVelocity = { 0.0f, 0.0f, 0.0f },
        float mass = 1.0f)
    {
        EntityId e = world->CreateEntity();

        // Transform
        TransformComponent& t = world->AddComponent<TransformComponent>(e);
        t.local.SetPosition(position);
        t.local.SetScale({ 1.0f, 1.0f, 1.0f });

        MeshRenderer& mr = world->AddComponent<MeshRenderer>(e);
        mr.geoId = RessourceManager::GetGeometryId("Cube");

        // Shape
        ShapeComponent& s = world->AddComponent<ShapeComponent>(e);
        s.SetBox(halfExtents);

        // RigidBody
        RigidBodyComponent& r = world->AddComponent<RigidBodyComponent>(e);
        r.SetMass(mass);
        r.type = BodyType::Dynamic;
        r.useGravity = true;
        r.allowRotation = true;

        // Motion
        MotionComponent& m = world->AddComponent<MotionComponent>(e);
		m.linearVelocity = initialVelocity;

        return e;
    }

    // ─── Corps statique (sol) ────────────────────────────────────────────────────
    static EntityId CreateStaticBox(World* world, const XMFLOAT3& position, const XMFLOAT3& halfExtents)
    {
        EntityId e = world->CreateEntity();

        TransformComponent& t = world->AddComponent<TransformComponent>(e);
        t.local.SetPosition(position);
		t.local.SetScale({ 10.0f, 1.0f, 10.0f });

		MeshRenderer& mr = world->AddComponent<MeshRenderer>(e);
        mr.geoId = RessourceManager::GetGeometryId("Cube");

        ShapeComponent& s = world->AddComponent<ShapeComponent>(e);
        s.SetBox(halfExtents);

        RigidBodyComponent& r = world->AddComponent<RigidBodyComponent>(e);
        r.SetStatic();  // massInverse = 0, tenseurs = 0

        world->AddComponent<MotionComponent>(e);

        return e;
    }

    // ─── Sphère dynamique ────────────────────────────────────────────────────────
    static EntityId CreateDynamicSphere(World* world, const XMFLOAT3& position, float radius, float mass = 1.0f)
    {
        EntityId e = world->CreateEntity();

        TransformComponent& t = world->AddComponent<TransformComponent>(e);
        t.local.SetPosition(position);

        MeshRenderer& mr = world->AddComponent<MeshRenderer>(e);
        mr.geoId = RessourceManager::GetGeometryId("Sphere");

        ShapeComponent& s = world->AddComponent<ShapeComponent>(e);
        s.SetSphere(radius * 0.5f);

        RigidBodyComponent& r = world->AddComponent<RigidBodyComponent>(e);
        r.SetMass(mass);
        r.type = BodyType::Dynamic;
        r.useGravity = true;

        world->AddComponent<MotionComponent>(e);

        return e;
    }

    // ─── Trigger (zone de détection) ────────────────────────────────────────────
    static EntityId CreateTrigger(World* world,
        const XMFLOAT3& position,
        const XMFLOAT3& halfExtents)
    {
        EntityId e = world->CreateEntity();

        TransformComponent& t = world->AddComponent<TransformComponent>(e);
        t.local.SetPosition(position);

        ShapeComponent& s = world->AddComponent<ShapeComponent>(e);
        s.SetBox(halfExtents);
        s.isTrigger = true;  // pas de réponse physique

        // Pas de RigidBodyComponent ni MotionComponent —
        // la broad phase et la narrow phase le traitent quand même
        // car elles ne demandent que ShapeComponent + TransformComponent.

        return e;
    }

    static void Run()
    {
        EngineManager::GetInstance().Initialize(1080, 720, L"Test Collision Reference");

        Scene* scene = SceneManager::GetSceneWithName("Default");
        World* world = scene->world;

        RessourceManager::AddGeometry("Cube", GeometryFactory::BuildCube(EngineManager::GetDevice()));
		RessourceManager::AddGeometry("Sphere", GeometryFactory::BuildIcosphere(EngineManager::GetDevice(), 10));
        uint32 shaderId = RessourceManager::AddShader("LitColored", ShaderFactory::CreateLitColored(EngineManager::GetDevice()));
        Material* white = RessourceManager::GetShader(shaderId)->CreateMaterial();
        RessourceManager::AddMaterial("White", white);

        RessourceManager::AddCamera("Default");

        ComponentRegistry::RegisterScript<CameraScript>();

        uint32 mat = RessourceManager::GetMaterialId("White");
		uint32 otherMat = RessourceManager::GetMaterialId("Default");

		CreateStaticBox(world, { 0.0f, -2.0f, 0.0f }, { 0.5f, 0.5f, 0.5f }); // sol
        //CreateDynamicBox(world, { 0.0f, 1.0f, 0.0f }, { 0.5f, 0.5f, 0.5f }, { 0.0f, 0.0f, 0.0f }, 2.0f); // cube dynamique
		CreateDynamicBox(world, { -1.0f, 0.0f, 0.0f }, { 0.5f, 0.5f, 0.5f }, { 1.0f, 0.0f, 0.0f }, 2.0f); // cube dynamique
		//CreateDynamicSphere(world, { 5.0f, 1.0f, 0.0f }, 1.0f, 2.0f); // sphère dynamique

        // -----------------------------
        // CAMERA
        // -----------------------------
        EntityId camera = world->CreateEntity();
        TransformComponent& tCamera = world->AddComponent<TransformComponent>(camera);
        tCamera.local.SetPosition(XMFLOAT3(0.0f, 4.0f, -19.0f));

        CameraComponent& cam = world->AddComponent<CameraComponent>(camera);
        cam.camId = RessourceManager::GetCameraId("Default");
        cam.isMainCamera = true;

        world->AddScript<CameraScript>(camera);

        // -----------------------------
        // LIGHT
        // -----------------------------
        EntityId light = world->CreateEntity();
        TransformComponent& tLight = world->AddComponent<TransformComponent>(light);
        tLight.local.SetPosition(XMFLOAT3(8.0f, 12.0f, -4.0f));

        LightComponent& l = world->AddComponent<LightComponent>(light);
        l.SetPoint(1.0f, 50.0f, 1);

        EngineManager::GetInstance().Run();
    }
};

#endif