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
	bool canResend;
};

class INetworkBase
{
public:
	INetworkBase();

	virtual void Update(float _dt) = 0;
	virtual void SendPackets() = 0;
	virtual void RegisterPacket(Packet _packet) = 0;
	virtual void HandlePacket() = 0;

	Sockets* GetSocket() const { return m_socket; }

	virtual ~INetworkBase() = default;
	
protected:
	const static inline float ACK_DELAY = 1.0f;
	const static inline uint8 MAX_RETRIES = 10;
	static inline uint16 ACK_COUNT = 0;
	
	Sockets* m_socket;
	char m_buffer[sizeof(Packet)] = {};
	Vector<PendingPacket> m_pendingPackets;
	Vector<Packet> m_packets;

	CriticalSection m_packetProtection;
	
	void SendReliablePacket(Packet _packet, sockaddr_in _target);
	void TickAck(float _deltaTime);
	void OnAckReceived(uint16 _ackId, sockaddr_in _sender);
	void SendAck(uint16 _ackId, sockaddr_in _target);
};

#endif // !I_NETWORK_BASE_H_DEFINED

