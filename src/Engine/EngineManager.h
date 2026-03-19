#ifndef ENGINE_MANAGER_H_DEFINED
#define ENGINE_MANAGER_H_DEFINED

// Engine
#include "SceneManager.h"
#include "define.h"

// Render
#include "Core/Chrono.h"

class Device;
class Window;
class Scene;
class Camera;
class RessourceManager;

class EngineManager
{
public:
    EngineManager();
    ~EngineManager();

    static EngineManager& GetInstance();
    
    void Initialize(UINT _width, UINT _height, WString _title, uint8 _flag = NetworkFlag::None);
    void Run();
    void Exit();
    
    static float GetDeltaTime() { return GetInstance().m_deltaTime; }
    static Window* GetWindow() { return GetInstance().m_pWindow; }
    static Device* GetDevice() { return GetInstance().m_pDevice; }
	static NetworkFlag GetNetworkFlag() { return GetInstance().m_networkFlag; }

private:
    static EngineManager* s_pInstance;

    Chrono m_chrono;
    float m_deltaTime = 0.0f;
    
    Camera* m_camera = nullptr;
    
    Window* m_pWindow = nullptr;
    Device* m_pDevice = nullptr;
    
    SceneManager* m_pSceneManager;
    RessourceManager* m_pRessourceManager;
    
    float m_DeltaTime = 0.0f;

	NetworkFlag m_networkFlag = NetworkFlag::None;
};

#endif

