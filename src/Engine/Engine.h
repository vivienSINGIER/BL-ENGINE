#ifndef ENGINE_H_DEFINED
#define ENGINE_H_DEFINED

// -------------- ECS ---------------
#include "ECS/World.h"
#include "ECS/ISystem.hpp"
#include "ECS/EntityManager.h"
#include "ECS/Script.hpp"

// ------------- ENGINE -------------
#include "EngineManager.h"
#include "RessourceManager.h"
#include "SceneManager.h"
#include "InputManager.h"

// ----------- COMPONENTS -----------
#include "Components/MeshRenderer.hpp"
#include "Components/TransformComponent.hpp"
#include "Components/CameraComponent.hpp"
#include "Components/ColliderComponent.hpp"

// ------------ SYSTEMS -------------
#include "Systems/MeshRendererSystem.h"
#include "Systems/TransformSystem.h"
#include "Systems/CameraSystem.h"
#include "Systems/ColliderSystem.h"

#endif