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

    static EntityId CreateCube(World* world,
        const XMFLOAT3& position,
        const XMFLOAT3& scale,
        uint32 material,
        bool dynamicBody,
        float mass = 1.0f,
        bool useGravity = true,
        bool rotation = false,
        const XMFLOAT3& velocity = XMFLOAT3(0.0f, 0.0f, 0.0f),
        float staticFriction = 0.5f,
        float dynamicFriction = 0.3f,
        float restitution = 0.0f)
    {
        EntityId e = world->CreateEntity();

        TransformComponent& t = world->AddComponent<TransformComponent>(e);
        t.local.SetPosition(position);
        t.local.SetScale(scale);

        ColliderComponent& c = world->AddComponent<ColliderComponent>(e);
        c.type = ColliderType::Box;

        PhysicComponent& p = world->AddComponent<PhysicComponent>(e);

        if (dynamicBody)
        {
            p.type = BodyType::Dynamic;
            p.SetMass(mass);
            p.useGravity = useGravity;
            p.rotation = rotation;
            p.velocity = velocity;
            p.staticFriction = staticFriction;
            p.dynamicFriction = dynamicFriction;
            p.restitution = restitution;
        }
        else
        {
            p.SetStatic();
            p.staticFriction = staticFriction;
            p.dynamicFriction = dynamicFriction;
            p.restitution = restitution;
        }

        MeshRenderer& m = world->AddComponent<MeshRenderer>(e);
		m.geoId = RessourceManager::GetGeometryId("Cube");
        if (material)
			m.materialId = material;

        return e;
    }

    static void Run()
    {
        EngineManager::GetInstance().Initialize(1080, 720, L"Test Collision Reference");

        Scene* scene = SceneManager::GetSceneWithName("Default");
        World* world = scene->world;

        RessourceManager::AddGeometry("Cube", GeometryFactory::BuildCube(EngineManager::GetDevice()));
        uint32 shaderId = RessourceManager::AddShader("LitColored", ShaderFactory::CreateLitColored(EngineManager::GetDevice()));
        Material* white = RessourceManager::GetShader(shaderId)->CreateMaterial();
        RessourceManager::AddMaterial("White", white);

        RessourceManager::AddCamera("Default");

        ComponentRegistry::RegisterScript<CameraScript>();

        uint32 mat = RessourceManager::GetMaterialId("White");
		uint32 otherMat = RessourceManager::GetMaterialId("Default");

        // -----------------------------
        // SOL PRINCIPAL
        // -----------------------------
        CreateCube(
            world,
            XMFLOAT3(0.0f, -1.0f, 0.0f),
            XMFLOAT3(80.0f, 1.0f, 80.0f),
            mat,
            false,
            1.0f,
            false,
            false,
            XMFLOAT3(0, 0, 0),
            0.8f,
            0.6f,
            0.0f
        );

        // -----------------------------
        // TEST A : chute simple
        // -----------------------------
        CreateCube(
            world,
            XMFLOAT3(-15.0f, 5.0f, 0.0f),
            XMFLOAT3(1.0f, 1.0f, 1.0f),
            otherMat,
            true,
            1.0f,
            true,
            false,
            XMFLOAT3(0, 0, 0),
            0.0f,
            0.0f,
            0.0f
        );

        // -----------------------------
        // TEST B : glissement sans friction
        // -----------------------------
        CreateCube(
            world,
            XMFLOAT3(-8.0f, 0.0f, 0.0f),
            XMFLOAT3(1.0f, 1.0f, 1.0f),
            otherMat,
            true,
            1.0f,
            true,
            false,
            XMFLOAT3(-3.0f, 0.0f, 0.0f),
            0.0f,
            0.0f,
            0.0f
        );

        // -----------------------------
        // TEST C : glissement avec friction
        // -----------------------------
        CreateCube(
            world,
            XMFLOAT3(-8.0f, 0.0f, -2.0f),
            XMFLOAT3(1.0f, 1.0f, 1.0f),
            otherMat,
            true,
            1.0f,
            true,
            false,
            XMFLOAT3(-3.0f, 0.0f, 0.0f),
            0.5f,
            0.3f,
            0.0f
        );

        // -----------------------------
        // TEST D : empilement
        // -----------------------------
        CreateCube(world, XMFLOAT3(8.0f, 0.0f, 0.0f), XMFLOAT3(1, 1, 1), otherMat, true, 1.0f, true, true);
        CreateCube(world, XMFLOAT3(8.0f, 2.1f, 0.0f), XMFLOAT3(1, 1, 1), otherMat, true, 1.0f, true, true);
        //CreateCube(world, XMFLOAT3(8.0f, 2.2f, 0.0f), XMFLOAT3(1, 1, 1), nullptr, true, 1.0f, true, true);

        // -----------------------------
        // TEST E : collision lat�rale
        // -----------------------------
        CreateCube(
            world,
            XMFLOAT3(0.0f, 0.0f, 6.0f),
            XMFLOAT3(1.0f, 1.0f, 1.0f),
            otherMat,
            true,
            1.0f,
            false,
            false,
            XMFLOAT3(5.0f, 0.0f, 0.0f),
            0.0f,
            0.0f,
            0.0f
        );

        CreateCube(
            world,
            XMFLOAT3(4.0f, 0.0f, 6.0f),
            XMFLOAT3(1.0f, 1.0f, 1.0f),
            otherMat,
            true,
            1.0f,
            false,
            false,
            XMFLOAT3(0.0f, 0.0f, 0.0f),
            0.0f,
            0.0f,
            0.0f
        );

        // -----------------------------
		// TEST F : Rotation
        // -----------------------------
        CreateCube(world, XMFLOAT3(0.0f, 1.0f, -4.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), otherMat, true, 1.0f, true, true, XMFLOAT3(6.0f, 0.0f, 0.0f), 0.0f, 0.0f, 0.0f);
        CreateCube(world, XMFLOAT3(3.0f, 0.0f, -4.5f), XMFLOAT3(1.0f, 1.0f, 1.0f), otherMat, true, 1.0f, true, true, XMFLOAT3(0.0f, 0.0f, 0.0f), 0.0f, 0.0f, 0.0f);

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