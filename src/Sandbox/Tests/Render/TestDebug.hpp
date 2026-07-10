#ifndef TEST_DEBUG_H_DEFINED
#define TEST_DEBUG_H_DEFINED

#include "Test.hpp"
#include "../Render/Generic/Render.h"
#include "Core/Math/Matrix/Matrix.h"
#include "Core/Math/Vector/Vector.h"
#include "Render/Generic/FontRendering/Text.hpp"

class TestDebug : public Test
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
    
    void Run()
    {
        Window window(1080, 720, L"Test", false);
        window.InitD3D12();

        Device* pDevice = window.GetDevice();

        XMFLOAT4X4 matrix = MathHelper::Identity4x4();

        Shader* s = ShaderFactory::CreateUnlitColored(pDevice);
        Material* green = s->CreateMaterial();
        green->SetFloat4("Color", {0.0f, 1.0f, 0.0f, 1.0f});

        Geometry* line = GeometryFactory::BuildLine(pDevice);
        Geometry* cube = GeometryFactory::BuildCube(pDevice);

        Camera cam;
        XMFLOAT3 pos = XMFLOAT3(0.0f, 0.0f, -5.0f);
        cam.SetPos(pos);
        XMFLOAT3 target = XMFLOAT3(0.0f, 0.0f, 0.0f);
        cam.LookAt(target);
        
        pDevice->SetMainCamera(&cam);

        Mat4f32 m1 = Mat4f32::MakeLineToLineTransform(
            Vect3f32(0.0f, 0.0f, 0.0f), Vect3f32(1.0f, 0.0f, 0.0f),
            Vect3f32(0.0f, 1.0f, 0.0f), Vect3f32(2.0f, 0.0f, 0.0f) );

        Mat4f32 m2 = Mat4f32::MakeLineToLineTransform(
            Vect3f32(0.0f, 0.0f, 0.0f), Vect3f32(1.0f, 0.0f, 0.0f),
            Vect3f32(0.0f, 0.0f, 0.0f), Vect3f32(0.0f, 1.0f, 0.0f) );

        Mat4f32 m3 = Mat4f32::MakeLineToLineTransform(
            Vect3f32(0.0f, 0.0f, 0.0f), Vect3f32(1.0f, 0.0f, 0.0f),
            Vect3f32(0.0f, 0.0f, 0.0f), Vect3f32(2.0f, 0.0f, 0.0f));

        Vect4f32 a1 = Vect4f32(0.0f, 0.0f, 0.0f, 1.0f);
        Vect4f32 a2 = Vect4f32(1.0f, 0.0f, 0.0f, 1.0f);

        Vect4f32 a1m1 = (a1 * m1);
        Vect4f32 a2m1 = (a2 * m1);
        Vect4f32 a1m2 = (a1 * m2);
        Vect4f32 a2m2 = (a2 * m2);
        Vect4f32 a1m3 = (a1 * m3);
        Vect4f32 a2m3 = (a2 * m3);
        
        std::cout << "a1 * M1 = " << a1m1<< std::endl;
        std::cout << "a2 * M1 = " << a2m1 << std::endl;
        std::cout << "a1 * M2 = " << a1m2 << std::endl;
        std::cout << "a2 * M2 = " << a2m2 << std::endl;
        std::cout << "a1 * M3 = " << a1m3 << std::endl;
        std::cout << "a2 * M3 = " << a2m3 << std::endl;
        
        XMFLOAT4X4 m1D3D = ToD3DMatrix(m1);

        XMFLOAT4X4 m2D3D = ToD3DMatrix(m2);

        XMFLOAT4X4 m3D3D = ToD3DMatrix(m3);
        
        while (window.IsOpen())
        {
            window.Update();
            window.Clear();

            pDevice->SetMaterial(green);
            pDevice->Draw(line, m1D3D);
            pDevice->Draw(line, m2D3D);
            pDevice->Draw(line, m3D3D);
            
            window.Display();
        }
    }
};

#endif