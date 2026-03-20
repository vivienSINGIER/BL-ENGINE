#ifndef NETWORK_SYSTEM_H_DEFINED
#define NETWORK_SYSTEM_H_DEFINED

#include "Components/NetworkComponent.hpp"
#include "../ECS/ISystem.hpp"

class Server;
class Client;

struct NetworkSystem : public System<NetworkComponent>
{
    void OnInit() override;
    
    void OnStartUpdate(float _dt) override;
    void OnUpdate(float _dt, EntityId id, NetworkComponent& _networkC) override;
    void OnEndUpdate(float _dt) override;

    void HandleClientReceive();
    void HandleServerReceive();
    
private:
    Server* m_server = nullptr;
    Client* m_client = nullptr;
};

#endif

