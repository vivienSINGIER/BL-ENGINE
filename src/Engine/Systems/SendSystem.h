#ifndef SEND_SYSTEM_H_DEFINED
#define SEND_SYSTEM_H_DEFINED

#include "../ECS/ISystem.h"
#include "../Components/NetworkComponent.hpp"

class Server;
class Client;

struct SendSystem : public System<NetworkComponent>
{
    void OnInit() override;
    
    void OnStartUpdate(float _dt) override;
    void OnUpdate(float _dt, EntityId _e, NetworkComponent& _n) override;
    void OnEndUpdate(float _dt) override;

private:
    Server* m_server = nullptr;
    Client* m_client = nullptr;
};

#endif

