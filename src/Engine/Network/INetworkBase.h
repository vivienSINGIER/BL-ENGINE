#ifndef I_NETWORK_BASE_H_DEFINED
#define I_NETWORK_BASE_H_DEFINED

#include "Engine.h"
#include "Sockets.h"
#include "Packet.hpp"
#include "CriticalSection.h"


struct PendingPacket
{
	Packet packet;
	sockaddr_in target;
	uint16 ackId;
	float timer;
	uint8 retryCount;
};

class INetworkBase
{
public:
	INetworkBase();

	virtual void SendPacket(Packet _packet) = 0;
	virtual void HandlePacket() = 0;

	Sockets* GetSocket() const { return m_socket; }

protected:
	void SendReliablePacket(Packet _packet, sockaddr_in _target);
	void TickAck(float _deltaTime);
	void OnAckReceived(uint16 _ackId);
	void SendAck(uint16 _ackId, sockaddr_in _target);

	Sockets* m_socket;
	Byte m_buffer[1024] = {};
	std::vector<PendingPacket> m_pendingPackets;

	CriticalSection m_packetProtection;

};

#endif // !I_NETWORK_BASE_H_DEFINED

