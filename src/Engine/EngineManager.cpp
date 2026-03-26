#ifndef ENGINE_MANAGER_CPP_DEFINED
#define ENGINE_MANAGER_CPP_DEFINED

#include "EngineManager.h"
#include "Engine.h"

#include "../Render/Generic/Render.h"
#include "../Render/Generic/Factories/ShaderFactory.hpp"
#include "ECS/SystemScheduler.h"
#include "Network/Client.h"
#include "Network/Server.h"

EngineManager* EngineManager::s_pInstance = nullptr;

EngineManager::EngineManager()
{
    s_pInstance = this;
    m_chrono = Chrono();
}

EngineManager::~EngineManager()
{
    s_pInstance->Exit();
}

EngineManager& EngineManager::GetInstance()
{
    if (s_pInstance == nullptr)
        s_pInstance = new EngineManager();

    return *s_pInstance;
}

void EngineManager::Initialize(UINT _width, UINT _height, WString _title, uint8 _flag)
{
    if (m_pWindow == nullptr)
    {
        m_pWindow = new Window(_width, _height, _title);
        m_pWindow->InitD3D12();
        m_pDevice = m_pWindow->GetDevice();
    }
    
    //m_pDevice->SetClearColor(ToColor(3, 63, 153)); //TO DO
    m_pRessourceManager = new RessourceManager;

    Shader* coloredS = ShaderFactory::CreateLitColored(m_pDevice);
    RessourceManager::AddShader("Color", coloredS);

    Material* white = coloredS->CreateMaterial();
    white->SetFloat4("DiffuseAlbedo", {1.0f, 1.0f, 1.0f, 1.0f});
    RessourceManager::AddMaterial("Default", white);

	m_networkFlag = static_cast<NetworkFlag>(_flag);

    if ((_flag & NetworkFlag::CLIENT) == NetworkFlag::CLIENT)
    {
        m_pClient = new Client();
        m_pClient->Init();
    }
    
	InputManager::Initialize(m_pWindow->GetHWND());
    if (m_pSceneManager == nullptr)
        m_pSceneManager = new SceneManager;

    ComponentRegistry::Init();

    SystemScheduler::Get().RegisterSystem<TransformSystem>(Phase::Update);
    SystemScheduler::Get().RegisterSystem<MeshRendererSystem>(Phase::Render);
    SystemScheduler::Get().RegisterSystem<CameraSystem>(Phase::PreRender);
    SystemScheduler::Get().RegisterSystem<LightSystem>(Phase::PreRender);
    SystemScheduler::Get().RegisterSystem<ReceiveSystem>(Phase::NetworkReceive);
    SystemScheduler::Get().RegisterSystem<SendSystem>(Phase::NetworkSend);
    SystemScheduler::Get().RegisterSystem<ColliderSystem>(Phase::FixedUpdate);
}

void EngineManager::Run()
{
    m_chrono.Start();
    
    while ( m_pWindow->IsOpen() )
    {
        m_deltaTime = m_chrono.Reset();
        
        m_pWindow->Update();
        InputManager::HandleInput();
        m_pSceneManager->GetCurrentScene()->Update(m_deltaTime);
    }
}

void EngineManager::Exit()
{
    delete m_pRessourceManager;
}

void EngineManager::HostServer(int _port)
{
    m_pServer = new Server();
    m_networkFlag |= NetworkFlag::SERVER;

    // TODO Check & Force server and connexion success
    
    m_pServer->Initialize("127.0.0.1", _port);

    Packet packet;
    packet.header.type = PacketType::Connect;
    packet.connect.addr = m_pClient->GetSocket()->GetAddr();
    
    m_pClient->SendReliablePacket(packet, m_pServer->GetSocket()->GetAddr());
}

void EngineManager::Connect(String const& _ip, int _port)
{
    sockaddr_in target;
    if ( inet_pton(AF_INET, _ip.c_str(), &target.sin_addr)<=0 )
    {
        assert(false && "Unvalid Server address");
    }
    target.sin_family = AF_INET;
    target.sin_port = htons(_port);
    
    Packet packet;
    packet.header.type = PacketType::Connect;
    packet.connect.addr = target;
    
    m_pClient->SendReliablePacket(packet, target);
}

#endif
