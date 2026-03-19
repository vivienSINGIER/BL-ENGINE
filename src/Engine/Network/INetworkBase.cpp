#include "INetworkBase.h"

INetworkBase::INetworkBase()
{
}

void INetworkBase::SendReliablePacket(Packet _packet, sockaddr_in _target)
{
}

void INetworkBase::TickAck(float _deltaTime)
{
}

void INetworkBase::OnAckReceived(uint16 _ackId)
{
}

void INetworkBase::SendAck(uint16 _ackId, sockaddr_in _target)
{
}
