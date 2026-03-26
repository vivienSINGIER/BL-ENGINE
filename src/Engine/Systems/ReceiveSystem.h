#ifndef RECEIVE_SYSTEM_H_DEFINED
#define RECEIVE_SYSTEM_H_DEFINED

#include "../ECS/ISystem.h"
#include "../Components/NetworkComponent.hpp"

class Server;
class Client;

struct ReceiveSystem : public System<NetworkComponent>
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

