#ifndef ENGINE_H_DEFINED
#define ENGINE_H_DEFINED

// -------------- ECS ---------------
#include "ECS/World.h"
#include "ECS/ISystem.h"
#include "ECS/EntityManager.h"
#include "ECS/Script.h"

// ------------- ENGINE -------------
#include "EngineManager.h"
#include "RessourceManager.h"
#include "SceneManager.h"
#include "InputManager.h"
#include "Scene.h"

// ----------- COMPONENTS -----------
#include "Components/MeshRenderer.hpp"
#include "Components/TransformComponent.hpp"
#include "Components/CameraComponent.hpp"
#include "Components/ColliderComponent.hpp"
#include "Components/LightComponent.hpp"
#include "Components/NetworkComponent.hpp"
#include "Components/OwnerComponent.hpp"
#include "Components/UiImageComponent.hpp"

// ------------ SYSTEMS -------------
#include "Systems/MeshRendererSystem.h"
#include "Systems/TransformSystem.h"
#include "Systems/CameraSystem.h"
#include "Systems/ColliderSystem.h"
#include "Systems/LightSystem.h"
#include "Systems/ReceiveSystem.h"
#include "Systems/SendSystem.h"

#endif