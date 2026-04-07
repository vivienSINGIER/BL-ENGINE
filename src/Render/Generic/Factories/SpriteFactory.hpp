#ifndef SPRITE_FACTORY_HPP_DEFINED
#define SPRITE_FACTORY_HPP_DEFINED

#include "../Base/Device.h"
#include "../RenderItems/Sprite.h"

class SpriteFactory
{
public:
    static Sprite* BuildRectangle(Device* _pDevice, int _width, int _height, bool _isDynamic = false)
    {
        Sprite* sprite = _pDevice->CreateSprite(_isDynamic);

        Vector<UiVertex> vertices;
        Vector<uint32> indices;

        float midWidth = (float)_width / 2;
        float midHeight = (float)_height / 2;
        
        vertices = {
            UiVertex(XMFLOAT2(-midWidth, midHeight), XMFLOAT2(0.0f, 0.0f)),
            UiVertex(XMFLOAT2(midWidth, midHeight), XMFLOAT2(1.0f, 0.0f)),
            UiVertex(XMFLOAT2(-midWidth, -midHeight), XMFLOAT2(0.0f, 1.0f)),
            UiVertex(XMFLOAT2(midWidth, -midHeight), XMFLOAT2(1.0f, 1.0f)),
        };

        indices = {
            0, 1, 2,
            2, 1, 3
        };

        sprite->SetVertexData(vertices.data(), vertices.size());
        sprite->SetIndexData(indices.data(), indices.size());

        return sprite;
    }

    static Sprite* BuildRoundedRectangle(Device* _pDevice, int _width, int _height, int _radius, bool _isDynamic = false)
    {
        Sprite* sprite = _pDevice->CreateSprite(_isDynamic);

        Vector<UiVertex> vertices;
        Vector<uint32> indices;

        float midWidth  = (float)_width  / 2;
        float midHeight = (float)_height / 2;
        float r         = (float)_radius;
        
        const int cornerSegments = 8;
        
        vertices.push_back(UiVertex(XMFLOAT2(0.0f, 0.0f), XMFLOAT2(0.5f, 0.5f)));
        
        XMFLOAT2 cornerCenters[4] = {
            XMFLOAT2(-midWidth + r,  midHeight - r),
            XMFLOAT2( midWidth - r,  midHeight - r),
            XMFLOAT2( midWidth - r, -midHeight + r),
            XMFLOAT2(-midWidth + r, -midHeight + r),
        };
        
        float startAngles[4] = {
            XM_PI,        
            XM_PIDIV2,    
            0.0f,           
            -XM_PIDIV2,     
        };

        uint32 perimeterStart = 1;
        for (int corner = 0; corner < 4; corner++)
        {
            for (int i = 0; i <= cornerSegments; i++)
            {
                float t     = (float)i / (float)cornerSegments;
                float angle = startAngles[corner] + t * XM_PIDIV2;

                float x = cornerCenters[corner].x + r * cosf(angle);
                float y = cornerCenters[corner].y + r * sinf(angle);
                
                float u = (x + midWidth)  / (float)_width;
                float v = 1.0f - (y + midHeight) / (float)_height;

                vertices.push_back(UiVertex(XMFLOAT2(x, y), XMFLOAT2(u, v)));
            }
        }
        
        uint32 perimeterCount = (uint32)vertices.size() - 1;

        for (uint32 i = 0; i < perimeterCount; i++)
        {
            uint32 curr = perimeterStart + i;
            uint32 next = perimeterStart + (i + 1) % perimeterCount;

            indices.push_back(0);
            indices.push_back(curr);
            indices.push_back(next);
        }

        sprite->SetVertexData(vertices.data(), (uint32)vertices.size());
        sprite->SetIndexData(indices.data(), (uint32)indices.size());

        return sprite;
    }
};

#endif