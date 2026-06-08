#ifndef TEST_WINDOW_H_DEFINED
#define TEST_WINDOW_H_DEFINED

#include "Test.hpp"
#include "../Render/Generic/Render.h"

class TestWindow : public Test
{
public: 
    static void Run()
    {
        Window window(1080, 720, L"Test");
        window.InitD3D12();

        Device* pDevice = window.GetDevice();
        
        Shader* coloredS = ShaderFactory::CreateLitColored(pDevice);

        Material* blue = coloredS->CreateMaterial();
        blue->SetFloat4("DiffuseAlbedo", XMFLOAT4(0.0f, 0.5f, 0.0f, 1.0f));
        blue->SetFloat("Roughness", 0.2f);

        Shader* textured = ShaderFactory::CreateUnlitTextured(pDevice);

        Material* brick = textured->CreateMaterial();
        Texture* brickTmp = pDevice->CreateTexture(RES("/Textures/Bricks/bricks.dds"));
        brick->SetTexture("Albedo", brickTmp);

        Shader* litTextured = ShaderFactory::CreateLitTextured(pDevice);

        Material* litBrick = litTextured->CreateMaterial();
        Texture* brickAlbedo = pDevice->CreateTexture(RES("/Textures/Concrete/Albedo.dds"));
        litBrick->SetTexture("Albedo", brickAlbedo);
        Texture* brickRoughness = pDevice->CreateTexture(RES("/Textures/Concrete/Roughness.dds"));
        litBrick->SetTexture("Roughness", brickRoughness);
        Texture* brickNormal = pDevice->CreateTexture(RES("/Textures/Concrete/Normal.dds"));
        litBrick->SetTexture("Normal", brickNormal);
        Texture* brickAmbient = pDevice->CreateTexture(RES("/Textures/Concrete/Ambient.dds"));
        litBrick->SetTexture("Ambient", brickAmbient);

        UiShader* uiShader = ShaderFactory::CreateUIBasic(pDevice);

        UiMaterial* uiColor = uiShader->CreateMaterial();
        uiColor->SetTexture("Image", brickTmp);
        
        Geometry* triangle = GeometryFactory::BuildDonut(pDevice, 1.0f, 0.5f, 32, 16, true);
        // Geometry* triangle = GeometryFactory::BuildUVSphere(pDevice, 16, 16);
        // Geometry* triangle = GeometryFactory::BuildIcosphere(pDevice, 5);
        // Geometry* triangle = GeometryFactory::BuildCube(pDevice);
        // Geometry* triangle = GeometryFactory::BuildPyramid(pDevice);
        // Geometry* triangle = GeometryFactory::BuildCylinder(pDevice, 16);

        Sprite* rect = SpriteFactory::BuildRectangle(pDevice, 200, 200);
        
        XMMATRIX geoMat = XMMatrixIdentity();
        XMMATRIX geoMat2 = XMMatrixIdentity();
        XMFLOAT4X4 uiMat = MathHelper::Identity4x4();

        XMFLOAT3 up = XMFLOAT3(1.0f, 1.0f, 0.0f);
        XMMATRIX rotationMat = XMMatrixRotationAxis(XMLoadFloat3(&up), -0.01f );

        Camera cam;
        XMFLOAT3 pos = XMFLOAT3(0.0f, 0.0f, -5.0f);
        cam.SetPos(pos);
        XMFLOAT3 target = XMFLOAT3(0.0f, 0.0f, 0.0f);
        cam.LookAt(target);

        float pitch = 0.0f;
        
        ComputeShader* luminanceCS = ComputeShaderFactory::CreateLuminance(pDevice);
        luminanceCS->SetRTV("Output", pDevice->GetBlitRT());
        
        pDevice->SetMainCamera(&cam);

        {
            LightDescriptor dirLight = LightHelper::CreateLight(LightType::Directional);
            dirLight.light.Direction = XMFLOAT3(-0.5f, 0.0f, 0.5f);
            dirLight.light.Position = XMFLOAT3(0.0f, 0.0f, 0.0f);
            dirLight.light.FalloffStart = 1.0f;
            dirLight.light.FalloffEnd = 20.f;
            dirLight.light.Strength = XMFLOAT3(2.0f, 2.0f, 2.0f);
            dirLight.light.SpotPower = 3.0f;
            dirLight.light.Color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
        
            Vector<LightDescriptor> lights = { dirLight };
            pDevice->SetLights(lights);
        }
        
        while (window.IsOpen())
        {
            window.Update();

            XMFLOAT4X4 mat;
            geoMat = XMMatrixMultiply(geoMat, rotationMat);
            XMStoreFloat4x4(&mat, geoMat);

            pitch += 0.01f;
            cam.SetRotation({pitch, 0.0f, 0.0f});
            
            window.Clear();
            
            pDevice->SetMaterial(blue);
            pDevice->Draw(triangle, mat);

            // pDevice->SetUiMaterial(uiColor);
            // pDevice->DrawUi(rect, uiMat); 

            pDevice->RunComputeShader(luminanceCS, true);

            window.Display();
        }
    }
};

#endif