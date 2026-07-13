#ifndef TEST_TRANSFORM_H_DEFINED
#define TEST_TRANSFORM_H_DEFINED

#include "Test.hpp"
#include "../Render/Generic/Render.h"
#include "Core/Math/Matrix/Matrix.h"
#include "Core/Math/Vector/Vector.h"
#include "Core/Transform.h"

#include "../Engine/InputManager.h"

class TestTransform : public Test
{
public:
    XMFLOAT4X4 ToD3DMatrix(Mat4f32 const& _m)
    {
        XMFLOAT4X4 M;

        M._11 = _m.m00;
        M._12 = _m.m01;
        M._13 = _m.m02;
        M._14 = _m.m03;

        M._21 = _m.m10;
        M._22 = _m.m11;
        M._23 = _m.m12;
        M._24 = _m.m13;

        M._31 = _m.m20;
        M._32 = _m.m21;
        M._33 = _m.m22;
        M._34 = _m.m23;

        M._41 = _m.m30;
        M._42 = _m.m31;
        M._43 = _m.m32;
        M._44 = _m.m33;
        return M;
    }
    
    void HandleObjectInput(InputManager& _im, Transform& t)
    {
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
        if (_im.IsKey(LCTRL, 1))
            t.Move(Vect3f32(0.0f, -1.0f, 0.0f) * dt );
        
        if (_im.IsKey(NUMPAD8, 1))
            t.AddYPR(Vect3f32(0.0f, 1.0f, 0.0f) * dt );
        if (_im.IsKey(NUMPAD5, 1))
            t.AddYPR(Vect3f32(0.0f, -1.0f, 0.0f) * dt );
        if (_im.IsKey(NUMPAD4, 1))
            t.AddYPR(Vect3f32(1.0f, 0.0f, 0.0f) * dt );
        if (_im.IsKey(NUMPAD6, 1))
            t.AddYPR(Vect3f32(-1.0f, 0.0f, 0.0f) * dt );
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
    
    void Run()
    {
        Window window(1080, 720, L"Test", false);
        window.InitD3D12();

        Device* pDevice = window.GetDevice();

        XMFLOAT4X4 matrix = MathHelper::Identity4x4();

        Shader* s = ShaderFactory::CreateLitColored(pDevice);
        Material* green = s->CreateMaterial();
        green->SetFloat4("DiffuseAlbedo", {0.0f, 1.0f, 0.0f, 1.0f});

        Geometry* line = GeometryFactory::BuildLine(pDevice);
        Geometry* cube = GeometryFactory::BuildCube(pDevice);

        Camera cam;
        XMFLOAT3 pos = XMFLOAT3(0.0f, 0.0f, -5.0f);
        cam.SetPos(pos);
        XMFLOAT3 target = XMFLOAT3(0.0f, 0.0f, 0.0f);
        cam.LookAt(target);
        
        pDevice->SetMainCamera(&cam);
        
        {
            LightDescriptor point1 = LightHelper::CreateLight(LightType::Point);
            point1.light.Position = XMFLOAT3(-2.0, -1.0f, -1.0f);
            point1.light.Strength = XMFLOAT3(1.0f, 1.0f, 1.0f);
            point1.light.Color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
        
            Vector<LightDescriptor> lights = { point1 };
            pDevice->SetLights(lights);
        }
        
        InputManager inputManager;
        inputManager.Initialize(window.GetHWND());
        
        Transform t;
        
        while (window.IsOpen())
        {
            HandleObjectInput(inputManager, t);
            matrix = ToD3DMatrix(t.GetMatrix());
            
            window.Update();
            window.Clear();

            pDevice->SetMaterial(green);
            pDevice->Draw(cube, matrix);
            
            window.Display();
        }
    }
};

#endif