#ifndef SPRITE_FACTORY_HPP_DEFINED
#define SPRITE_FACTORY_HPP_DEFINED

#include "../Base/Device.h"
#include "../RenderItems/Sprite.h"

class SpriteFactory
{
public:
    static Sprite* BuildRectangle(Device* _pDevice, int _width, int _height, bool _isDynamic = false)
    {
        Sprite* sprite = _pDevice->CreateSprite();

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
        Sprite* sprite = _pDevice->CreateSprite();

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
};

#endif