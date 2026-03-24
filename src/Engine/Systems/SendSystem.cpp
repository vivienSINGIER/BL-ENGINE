#include "SendSystem.h"

#include "EngineManager.h"
#include "Network/Client.h"
#include "Network/Server.h"

void SendSystem::OnInit()
{
    
}

void SendSystem::OnStartUpdate(float _dt)
{
    m_client = EngineManager::GetClient();
    m_server = EngineManager::GetServer();
    
    if (m_client != nullptr)
        m_client->Update(_dt);
    if (m_server != nullptr)
        m_server->Update(_dt);
}


