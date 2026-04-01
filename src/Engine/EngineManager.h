#ifndef ENGINE_MANAGER_H_DEFINED
#define ENGINE_MANAGER_H_DEFINED

// Engine
#include "define.h"

// Render
#include "Core/Chrono.h"

class Device;
class Window;
class Scene;
class Camera;
class RessourceManager;
class SceneManager;
class Client;
class Server;

class EngineManager
{
public:
    EngineManager();
    ~EngineManager();

    static EngineManager& GetInstance();
    
    void Initialize(UINT _width, UINT _height, WString _title, uint8 _flag = NetworkFlag::CLIENT);
    void Run();
    void Exit();

	void HostServer(int _port = 1888);
	void Connect(String const& ip, int _port);
	
    static float GetDeltaTime() { return GetInstance().m_deltaTime; }
    static Window* GetWindow() { return GetInstance().m_pWindow; }
    static Device* GetDevice() { return GetInstance().m_pDevice; }
	static uint8 GetNetworkFlag() { return GetInstance().m_networkFlag; }

	static bool		IsServer()	{ return GetInstance().m_pServer != nullptr; }
	static Client*	GetClient() { return GetInstance().m_pClient; }
	static Server*	GetServer() { return GetInstance().m_pServer; }

private:
    static EngineManager* s_pInstance;

    Chrono m_chrono;
    float m_deltaTime = 0.0f;
    
    Camera* m_camera = nullptr;
    
    Window* m_pWindow = nullptr;
    Device* m_pDevice = nullptr;
    
    SceneManager* m_pSceneManager;
    RessourceManager* m_pRessourceManager;

	Client* m_pClient = nullptr;
	Server* m_pServer = nullptr;
    
    float m_DeltaTime = 0.0f;

	uint8 m_networkFlag = NetworkFlag::NONE;
};

#endif

