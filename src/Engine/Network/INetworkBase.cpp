#include "INetworkBase.h"

INetworkBase::INetworkBase()
{
    m_socket = new Sockets(UDP);
}

void INetworkBase::SendReliablePacket(Packet _packet, sockaddr_in _target)
{
    PendingPacket pending;
    pending.packet     = _packet;
    pending.target     = _target;
    pending.ackId      = ACK_COUNT++;
    pending.timer      = 0.0f;
    pending.retryCount = 0;
    pending.canResend = true;

    m_pendingPackets.push_back(pending);
}

void INetworkBase::RegisterPacket(Packet _packet)
{
    m_packets.push_back(_packet);
}

Vector<Packet>& INetworkBase::GetReceived()
{
    return m_packets;
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
                continue;
            }
            
            pending.canResend = true;
        }
        i++;
    }
}

void INetworkBase::OnAckReceived(uint16 _ackId, sockaddr_in _sender)
{
    bool isSelf = false;
    int i = 0; 
    while (i < m_pendingPackets.size())
    {
        if (m_pendingPackets[i].ackId == _ackId)
        {
            isSelf = true;
            m_pendingPackets.erase(m_pendingPackets.begin() + i);
            break;
        }
        i++;
    }

    if (isSelf == false)
    {
        SendAck(_ackId, _sender);
    }
}

void INetworkBase::SendAck(uint16 _ackId, sockaddr_in _target)
{
    Packet packet;
    packet.header.ackId = _ackId;
    packet.header.type = PacketType::Ack;

    // TODO add tick in packet

    m_socket->Send(packet.Data(), packet.Size(), _target);
}
