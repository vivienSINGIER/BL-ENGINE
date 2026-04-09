#ifndef TEST_TEXT_H_DEFINED
#define TEST_TEXT_H_DEFINED

#include "Test.h"
#include "../Render/Generic/Render.h"
#include "Render/Generic/FontRendering/Text.hpp"

class TestText : public Test
{
public: 
    static void Run()
    {
        Window window(1080, 720, L"Test", true);
        window.InitD3D12();

        Device* pDevice = window.GetDevice();

        // RenderFont* font = pDevice->CreateRenderFont(RES("/Font/JetBrainsMono-Bold.ttf"), 150.0f);
        // RenderFont* font = pDevice->CreateRenderFont(RES("/Font/test.ttf"), 150.0f);
        RenderFont* font = pDevice->CreateRenderFont(RES("/Font/Valentine.ttf"), 150.0f);
        // RenderFont* font = pDevice->CreateRenderFont(RES("/Font/west.ttf"), 150.0f);
        
        Text* text = pDevice->CreateText(font);

        text->SetString("Tu est moche");

        XMFLOAT4X4 matrix = MathHelper::Identity4x4();
        matrix._41 = -300.0f;
        
        while (window.IsOpen())
        {
            window.Update();
            window.Clear();
            
            pDevice->DrawRenderText(text, matrix);
            
            window.Display();
        }
    }
};

#endif