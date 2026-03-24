#ifndef SEND_SYSTEM_H_DEFINED
#define SEND_SYSTEM_H_DEFINED

#include "../ECS/ISystem.hpp"
#include "../Components/NetworkComponent.hpp"

class Server;
class Client;

struct SendSystem : public System<NetworkComponent>
{
    void OnInit() override;
    
    void OnStartUpdate(float _dt) override;

private:
    Server* m_server = nullptr;
    Client* m_client = nullptr;
};

#endif

