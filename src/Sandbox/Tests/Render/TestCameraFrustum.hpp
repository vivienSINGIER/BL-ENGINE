#ifndef TEST_CAMERA_FRUSTUM_H_DEFINED
#define TEST_CAMERA_FRUSTUM_H_DEFINED

#include "Test.hpp"
#include "../Render/Generic/Render.h"
#include "Core/Math/Matrix/Matrix.h"
#include "Core/Math/Vector/Vector.h"
#include "Core/Transform.h"

#include "../Engine/InputManager.h"

#include "../Core/Math/Geometry/AABB.h"
#include "Core/Math/Geometry/Frustum.h"
#include "Core/Math/Geometry/OBB.h"
#include "Core/Math/Geometry/Ray.h"
#include "Core/Math/Geometry/Sphere.h"

class TestCameraFrustum : public Test
{
public:
    Geometry* cube;
    Geometry* sphere;
    Geometry* line;
    Camera cam;
    Transform camT;
    
    void HandleObjectInput(InputManager& _im, Transform& t)
    {
        if (_im.IsKey(LCONTROL, 1) == true)
            return;
        
        float dt = 1.0f / 60.0f;
        
        if (_im.IsKey(D, 1))
            t.Move(Vect3f32(1.0f, 0.0f, 0.0f) * dt );
        if (_im.IsKey(Q, 1))
            t.Move(Vect3f32(-1.0f, 0.0f, 0.0f) * dt );
        if (_im.IsKey(Z, 1))
            t.Move(Vect3f32(0.0f, 0.0f, 1.0f) * dt );
        if (_im.IsKey(S, 1))
            t.Move(Vect3f32(0.0f, 0.0f, -1.0f) * dt );
        if (_im.IsKey(SPACE, 1))
            t.Move(Vect3f32(0.0f, 1.0f, 0.0f) * dt );
        if (_im.IsKey(LSHIFT, 1))
            t.Move(Vect3f32(0.0f, -1.0f, 0.0f) * dt );
        
        if (_im.IsKey(NUMPAD8, 1))
            t.AddYPR(Vect3f32(0.0f, 1.0f, 0.0f) * dt );
        if (_im.IsKey(NUMPAD5, 1))
            t.AddYPR(Vect3f32(0.0f, -1.0f, 0.0f) * dt );
        if (_im.IsKey(NUMPAD4, 1))
            t.AddYPR(Vect3f32(-1.0f, 0.0f, 0.0f) * dt );
        if (_im.IsKey(NUMPAD6, 1))
            t.AddYPR(Vect3f32(1.0f, 0.0f, 0.0f) * dt );
        if (_im.IsKey(NUMPAD7, 1))
            t.AddYPR(Vect3f32(0.0f, 0.0f, 1.0f) * dt );
        if (_im.IsKey(NUMPAD9, 1))
            t.AddYPR(Vect3f32(0.0f, 0.0f, -1.0f) * dt );
        
        if (_im.IsKey(NUMPAD_ADD, 1))
            t.Scale( 1.01f );
        if (_im.IsKey(NUMPAD_SUBTRACT, 1))
            t.Scale( 0.99f );
        
        _im.HandleInput(1);
    }
    
    void HandleCameraInput(InputManager& _im)
    {
        if (_im.IsKey(LCONTROL, 1) == false)
            return;
        
        float dt = 1.0f / 60.0f;
        
        if (_im.IsKey(D, 1))
            camT.Move(Vect3f32(1.0f, 0.0f, 0.0f) * dt );
        if (_im.IsKey(Q, 1))
            camT.Move(Vect3f32(-1.0f, 0.0f, 0.0f) * dt );
        if (_im.IsKey(Z, 1))
            camT.Move(Vect3f32(0.0f, 0.0f, 1.0f) * dt );
        if (_im.IsKey(S, 1))
            camT.Move(Vect3f32(0.0f, 0.0f, -1.0f) * dt );
        if (_im.IsKey(SPACE, 1))
            camT.Move(Vect3f32(0.0f, 1.0f, 0.0f) * dt );
        if (_im.IsKey(LSHIFT, 1))
            camT.Move(Vect3f32(0.0f, -1.0f, 0.0f) * dt );
        
        if (_im.IsKey(NUMPAD8, 1))
            camT.AddYPR(Vect3f32(0.0f, 1.0f, 0.0f) * dt );
        if (_im.IsKey(NUMPAD5, 1))
            camT.AddYPR(Vect3f32(0.0f, -1.0f, 0.0f) * dt );
        if (_im.IsKey(NUMPAD4, 1))
            camT.AddYPR(Vect3f32(-1.0f, 0.0f, 0.0f) * dt );
        if (_im.IsKey(NUMPAD6, 1))
            camT.AddYPR(Vect3f32(1.0f, 0.0f, 0.0f) * dt );
        if (_im.IsKey(NUMPAD7, 1))
            camT.AddYPR(Vect3f32(0.0f, 0.0f, 1.0f) * dt );
        if (_im.IsKey(NUMPAD9, 1))
            camT.AddYPR(Vect3f32(0.0f, 0.0f, -1.0f) * dt );
        
        if (_im.IsKey(NUMPAD_ADD, 1))
            camT.Scale( 1.01f );
        if (_im.IsKey(NUMPAD_SUBTRACT, 1))
            camT.Scale( 0.99f );
        
        _im.HandleInput(1);
    }
        
    void Run()
    {
        Window window(1080, 720, L"Test", false);
        window.InitD3D12();

        Device* pDevice = window.GetDevice();

        Shader* s = ShaderFactory::CreateWireframe(pDevice);
        Material* greenWF = s->CreateMaterial();
        greenWF->SetFloat4("Color", {0.0f, 1.0f, 0.0f, 1.0f});
        Material* redWF = s->CreateMaterial();
        redWF->SetFloat4("Color", {1.0f, 0.0f, 0.0f, 1.0f});
        
        Shader* s2 = ShaderFactory::CreateLitColored(pDevice);
        Material* white = s2->CreateMaterial();
        white->SetFloat4("DiffuseAlbedo", {1.0f, 1.0f, 1.0f, 1.0f});

        line = GeometryFactory::BuildLine(pDevice);
        cube = GeometryFactory::BuildCube(pDevice);
        sphere = GeometryFactory::BuildIcosphere(pDevice, 2);
        
        camT.SetPosition(Vect3f32(0.0f, 0.0f, -5.0f));
        camT.LookAt({0.0f, 0.0f, 0.0f});
        
        cam.SetWorld(camT.GetMatrix());
        pDevice->SetMainCamera(&cam);
        
        {
            LightDescriptor point1 = LightHelper::CreateLight(LightType::Point);
            point1.light.Position = Vect3f32(-2.0, -1.0f, -1.0f);
            point1.light.Strength = Vect3f32(1.0f, 1.0f, 1.0f);
            point1.light.Color = Vect4f32(1.0f, 1.0f, 1.0f, 1.0f);
        
            Vector<LightDescriptor> lights = { point1 };
            pDevice->SetLights(lights);
        }
        
        InputManager inputManager;
        inputManager.Initialize(window.GetHWND());
        
        Transform t;
        t.SetScale({0.5f, 0.2f, 0.7f});
        Transform t0;
        t0.Scale(0.2f);
        
        Ray r = Ray(Vect3f32(0.0f), Vect3f32(1.0f, 0.0, 0.0f), 3.0f);
        
        Mat4f32 proj = Mat4f32::MakePerspective(0.5f, 16.f / 9.0f, 0.1f, 3.0f);
        
        while (window.IsOpen())
        {
            HandleObjectInput(inputManager, t);
            HandleCameraInput(inputManager);
            
            window.Update();
            window.Clear();

            cam.SetWorld(camT.GetMatrix());
            
            pDevice->SetMaterial(white);
            
            if (cube->FrustumCheck(cam.GetFrustum(), t.GetMatrix()))
            {
                pDevice->SetMaterial(redWF);
            }
            else
                pDevice->SetMaterial(greenWF);
            
            pDevice->Draw(sphere, t0.GetMatrix());
            pDevice->Draw(cube, t.GetMatrix());
            
            window.Display();
        }
    }
};

#endif