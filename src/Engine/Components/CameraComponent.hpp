#ifndef CAMERA_COMPONENT_HPP_DEFINED
#define CAMERA_COMPONENT_HPP_DEFINED

#include "../../Render/Generic/Render.h"

struct CameraComponent
{
    Camera camera;
    bool isMainCamera = false;
};

#endif
