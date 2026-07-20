#ifndef TEST_FRUSTUM_H_DEFINED
#define TEST_FRUSTUM_H_DEFINED

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

class TestFrustum : public Test
{
public:
    Geometry* cube;
    Geometry* sphere;
    Geometry* line;
    Camera cam;
    
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
    
    void HandleFrustumInput(InputManager& _im, Transform& t)
    {
        if (_im.IsKey(LCONTROL, 1) == false)
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
    
    void HandleCamInput(InputManager& _im)
    {
        if (_im.IsKeyDown(_1, 1))
        {
            XMFLOAT3 pos = XMFLOAT3(0.0f, 0.0f, -5.0f);
            cam.SetPos(pos);
        }
        if (_im.IsKeyDown(_2, 1))
        {
            XMFLOAT3 pos = XMFLOAT3(5.0f, 0.0f, 0.0f);
            cam.SetPos(pos); 
        }
        if (_im.IsKeyDown(_3, 1))
        {
            XMFLOAT3 pos = XMFLOAT3(0.1f, 5.0f, 0.0f);
            cam.SetPos(pos); 
        }
        
        XMFLOAT3 target = XMFLOAT3(0.0f, 0.0f, 0.0f);
        cam.LookAt(target);
    }
    
    void DrawLine(Device* _d, Vect3f32 _p1, Vect3f32 _p2)
    {
        Mat4f32 t = Mat4f32::MakeLineToLineTransform(Vect3f32(0.0f), Vect3f32(1.0f, 0.0f, 0.0f), _p1, _p2);
        
        XMFLOAT4X4 m = ToD3DMatrix(t);
        
        _d->Draw(line, m);
    }
    
    void DrawAABB(Device* _d, AABB _a)
    {
        Mat4f32 scale;
        scale.m00 = _a.Extent().x * 2.0f;
        scale.m11 = _a.Extent().y * 2.0f;
        scale.m22 = _a.Extent().z * 2.0f;
        scale.rows[3] = Vect4f32(_a.Center(), 1.0f);
        
        XMFLOAT4X4 m = ToD3DMatrix(scale);
        
        _d->Draw(cube, m);
    }
    
    void DrawSphere(Device* _d, Sphere _s)
    {
        Mat4f32 scale;
        scale.m00 = _s.radius * 2.0f;
        scale.m11 = _s.radius * 2.0f;
        scale.m22 = _s.radius * 2.0f;
        scale.rows[3] = Vect4f32(_s.center, 1.0f);
        
        XMFLOAT4X4 m = ToD3DMatrix(scale);
        
        _d->Draw(sphere, m);
    }
    
    void DrawOBB(Device* _d, OBB _o)
    {
        Mat4f32 t = Mat4f32::MakeTransform(_o.position, _o.extent * 2.0f, _o.orientation.ToMatrix4());
        
        XMFLOAT4X4 m = ToD3DMatrix(t);
        
        _d->Draw(cube, m);
    }
    
    void DrawFrustum(Device* _d, Frustum _f)
    {
        Vect3f32 points[8];
        Plane::ThreeWayIntersect(_f.nearZ, _f.left, _f.top, &points[0]);
        Plane::ThreeWayIntersect(_f.nearZ, _f.right, _f.top, &points[1]);
        Plane::ThreeWayIntersect(_f.nearZ, _f.right, _f.bottom, &points[2]);
        Plane::ThreeWayIntersect(_f.nearZ, _f.left, _f.bottom, &points[3]);
        
        Plane::ThreeWayIntersect(_f.farZ, _f.left, _f.top, &points[4]);
        Plane::ThreeWayIntersect(_f.farZ, _f.right, _f.top, &points[5]);
        Plane::ThreeWayIntersect(_f.farZ, _f.right, _f.bottom, &points[6]);
        Plane::ThreeWayIntersect(_f.farZ, _f.left, _f.bottom, &points[7]);
        
        for (int i = 0; i < 4; i++)
            DrawLine(_d, points[i], points[(i + 1) % 4]);
        
        for (int i = 0; i < 4; i++)
            DrawLine(_d, points[4 + i], points[4 + (i + 1) % 4]);
        
        for (int i = 0; i < 4; i++)
            DrawLine(_d, points[i], points[4 + i]);
    }
    
    void DrawRay(Device* _d, Ray _r)
    {
        Vect3f32 origin = _r.origin;
        Vect3f32 end = _r.origin + _r.direction * _r.length;
        
        Mat4f32 t = Mat4f32::MakeLineToLineTransform(
            Vect3f32(0.0f, 0.0f, 0.0f), Vect3f32(1.0f, 0.0f, 0.0f),
            origin, end);
        
        XMFLOAT4X4 m = ToD3DMatrix(t);
        
        _d->Draw(line, m);
    }
        
    void Run()
    {
        Window window(1080, 720, L"Test", false);
        window.InitD3D12();

        Device* pDevice = window.GetDevice();

        XMFLOAT4X4 matrix = MathHelper::Identity4x4();
        XMFLOAT4X4 fM = MathHelper::Identity4x4();

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
        Transform fT;
        AABB aabb = AABB(Vect3f32(-0.1), Vect3f32(0.1));
        float rad = Vect3f32(0.5f).Length();
        Sphere o = Sphere(Vect3f32(), 0.2f);
        OBB obb = OBB(Vect3f32(), Vect3f32(0.5f, 0.2f, 0.1f));
        
        Ray r = Ray(Vect3f32(0.0f), Vect3f32(1.0f, 0.0, 0.0f), 3.0f);
        
        Mat4f32 proj = Mat4f32::MakePerspective(0.5f, 16.f / 9.0f, 0.1f, 3.0f);
        
        while (window.IsOpen())
        {
            HandleObjectInput(inputManager, t);
            HandleFrustumInput(inputManager, fT);
            HandleCamInput(inputManager);
            
            matrix = ToD3DMatrix(t.GetMatrix());
            
            Mat4f32 view = fT.GetInvMatrix();
            Frustum frustum(view, proj);
            
            window.Update();
            window.Clear();

            pDevice->SetMaterial(white);
            DrawFrustum(pDevice, frustum);
            
            if (frustum.Intersects(obb.Transformed(t.GetMatrix())))
            {
                pDevice->SetMaterial(redWF);
                // intersect.SetPosition(intersectPos);
                //
                // XMFLOAT4X4 m = ToD3DMatrix(intersect.GetMatrix());
                //
                // pDevice->Draw(sphere, m);
            }
            else
                pDevice->SetMaterial(greenWF);
            
            DrawOBB(pDevice, obb.Transformed(t.GetMatrix()));
            // DrawAABB(pDevice, aabb);
            // DrawSphere(pDevice, o.Transformed(fT.GetMatrix()));
            // DrawSphere(pDevice, o);
            DrawSphere(pDevice, o.Transformed(fT.GetMatrix()));
            // DrawOBB(pDevice, obb);
            // DrawRay(pDevice, r);
            
            window.Display();
        }
    }
};

#endif