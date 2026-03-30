#include "Client.h"

Client::Client() : INetworkBase()
{
    m_isConnected = false;
    m_isRunning = true;
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
    for (PendingPacket& pending : m_pendingPackets)
    {
        if (pending.canResend)
        {
            GetSocket()->Send(pending.packet.Data(), pending.packet.Size(), pending.target);
            pending.canResend = false;
        }
    }
    
    if (m_isConnected == false)
        return;

    for (Packet packet : m_packets)
    {
        GetSocket()->Send(packet.Data(), packet.Size(), m_serverAddress);
    }
    m_packets.clear();
}

void Client::Connect(sockaddr_in _addr)
{
    m_serverAddress = _addr;
    m_isConnected = true;
}

DWORD Client::ReceiveThread(LPVOID _lpParam)
{
    Client* client = static_cast<Client*>(_lpParam);
    char buffer[BUFFER_SIZE] = {};
    sockaddr_in sender;
    while (client->m_isRunning)
    {
		memset(buffer, 0, BUFFER_SIZE);
        int bytesRead = client->GetSocket()->Receive(buffer, BUFFER_SIZE, sender);
        if (bytesRead > 0)
        {
            Packet packet;
            memcpy(&packet, buffer, bytesRead);

            ReceivedPacket receivedPacket;
            receivedPacket.packet = packet;
            receivedPacket.sender = sender;
			
            client->m_packetProtection.Enter();
            client->m_receivedPackets.push_back(receivedPacket);
            client->m_packetProtection.Leave();
        }
    }

    return 1;
}
