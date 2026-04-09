#ifndef UI_IMAGE_H_DEFINED
#define UI_IMAGE_H_DEFINED

#include "../../Core/Transform2D.h"

struct UiImageComponent
{
    uint32 spriteId;
    uint32 materialId;
    
    Transform2D transform;
};

#endif
