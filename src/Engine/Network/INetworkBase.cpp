#include "INetworkBase.h"

#include <stdexcept>

INetworkBase::INetworkBase()
{
    if (!SocketsMethods::Start())
        throw std::runtime_error("WSAStartup échoué");
    
    m_socket = new Sockets(UDP);
}

void INetworkBase::SendReliablePacket(Packet _packet, sockaddr_in _target)
{
    PendingPacket pending;
    pending.packet     = _packet;
    pending.target     = _target;
    pending.packet.header.ackId = ACK_COUNT++;
    pending.ackId = pending.packet.header.ackId;
    pending.timer      = 0.0f;
    pending.retryCount = 0;
    pending.canResend = true;

    m_pendingPackets.push_back(pending);
}

void INetworkBase::RegisterPacket(Packet _packet)
{
    m_packets.push_back(_packet);
}

Vector<ReceivedPacket>& INetworkBase::GetReceived()
{
    return m_receivedPackets;
}

void INetworkBase::ClearReceived()
{
    m_packetProtection.Enter();
    m_receivedPackets.clear();
    m_packetProtection.Leave();
}

void INetworkBase::TickAck(float _deltaTime)
{
    int i = 0;
    while (i < m_pendingPackets.size())
    {
        PendingPacket& pending = m_pendingPackets[i];
        pending.timer += _deltaTime;

        if (pending.timer >= ACK_DELAY)
        {
            pending.timer = 0.0f;
            pending.retryCount++;

            if (pending.retryCount >= MAX_RETRIES)
            {
                m_pendingPackets.erase(m_pendingPackets.begin() + i);
                std::cout << "Cleared pending message :" << pending.ackId << std::endl;
                continue;
            }
            
            pending.canResend = true;
        }
        i++;
    }
}

void INetworkBase::OnAckReceived(uint16 _ackId)
{
    bool isSelf = false;
    int i = 0;
    while (i < m_pendingPackets.size())
    {
        if (m_pendingPackets[i].ackId == _ackId)
        {
            isSelf = true;
            std::cout << "Cleared pending message :" << m_pendingPackets[i].ackId << std::endl;
            m_pendingPackets.erase(m_pendingPackets.begin() + i);
            break;
        }
        i++;
    }
}

void INetworkBase::SendAck(uint16 _ackId, sockaddr_in _target)
{
    Packet packet;
    packet.header.ackId = _ackId;
    packet.header.type = PacketType::Ack;

    // TODO add tick in packet
    std::cout << "Send ack :" << _ackId << std::endl;
    m_socket->Send(packet.Data(), packet.Size(), _target);
}
