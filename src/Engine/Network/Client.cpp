#include "Client.h"

Client::Client()
{
    m_isConnected = false;
    m_isRunning = false;
    m_serverAddress = {};   
}

void Client::Init()
{
    CreateThread(NULL, 0, ReceiveThread, this, 0, NULL);
}

void Client::Update(float _dt)
{
    TickAck(_dt);
    m_tickAccumulator += _dt;

    if (m_tickAccumulator < TICK_DELAY)
        return;

    m_tickAccumulator -= TICK_DELAY;

    SendPackets();
}

void Client::SendPackets()
{
    for (PendingPacket pending : m_pendingPackets)
    {
        if (pending.canResend)
        {
            GetSocket()->Send(pending.packet.Data(), BUFFER_SIZE, pending.target);
        }
    }
    
    if (m_isConnected == false)
        return;

    for (Packet packet : m_packets)
    {
        GetSocket()->Send(packet.Data(), BUFFER_SIZE, m_serverAddress);
    }
    m_packets.clear();
}

DWORD Client::ReceiveThread(LPVOID _lpParam)
{
    Client* client = static_cast<Client*>(_lpParam);
    char buffer[BUFFER_SIZE] = {};
    sockaddr_in sender;
    while (client->m_isRunning)
    {
        int bytesRead = client->GetSocket()->Receive(buffer, BUFFER_SIZE, sender);
        if (bytesRead > 0)
        {
            Packet packet;
            memcpy(&packet, buffer, BUFFER_SIZE);

            client->m_packetProtection.Enter();
            client->m_receivedPackets.push_back(packet);
            client->m_packetProtection.Leave();
        }
    }

    return 1;
}
