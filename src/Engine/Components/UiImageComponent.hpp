#ifndef MESH_RENDERER_H_DEFINED
#define MESH_RENDERER_H_DEFINED

#include "../../Core/Transform2D.h"
#include "../Render/Generic/Render.h"

struct UiImage
{
    uint32 spriteId;
    uint32 materialId;
    
    Transform2D transform;
};

#endif
