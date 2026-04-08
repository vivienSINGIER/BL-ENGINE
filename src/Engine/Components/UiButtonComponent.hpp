#ifndef UI_BUTTON_H_DEFINED
#define UI_BUTTON_H_DEFINED

#include "../../Core/Transform2D.h"

struct UiButtonComponent
{
    enum ButtonStateType
    {
        IDLE, HOVERED, PRESSED, DISABLED
    };
    
    struct ButtonState
    {
        uint32 materialId;

        XMFLOAT2 statePos = {0.0f, 0.0f};
        XMFLOAT2 stateScale = {1.0f, 1.0f};
        float stateRotation = 0.0f;
    };
    
    uint32 spriteId;
    Transform2D transform;

    ButtonStateType currState = IDLE;
    Array<ButtonState, 4> states;

    bool isPressed = false;
    float pressCooldown = 0.0f;
    float pressAccumulator = 0.0f;

    bool isHovered = false;
    bool isDisabled = false;

private:
    bool m_isInit = false;
    int m_halfWidth = 0;
    int m_halfHeight = 0;
    
    friend struct ButtonSystem;
};

#endif
