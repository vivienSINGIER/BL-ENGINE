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

	void SendReliablePacket(Packet _packet, sockaddr_in _target);
	void RegisterPacket(Packet _packet);

	CriticalSection& GetCritSection() { return m_packetProtection; }
	
	Sockets* GetSocket() const { return m_socket; }
	Vector<Packet>& GetReceived();
	void ClearReceived();
	
	virtual ~INetworkBase() = default;
	
protected:
	constexpr static int BUFFER_SIZE = sizeof(Packet);
	const static inline float ACK_DELAY = 5.0f;
	const static inline uint8 MAX_RETRIES = 10;
	const static inline float TICK_DELAY = 0.01666666666f;
	static inline uint16 ACK_COUNT = 1;

	float m_tickAccumulator = 0.0f;
	
	Sockets* m_socket;
	char m_buffer[BUFFER_SIZE] = {};
	Vector<PendingPacket> m_pendingPackets;
	Vector<Packet> m_packets;
	Vector<Packet> m_receivedPackets;

	CriticalSection m_packetProtection;
	
	void TickAck(float _deltaTime);
	void OnAckReceived(uint16 _ackId, sockaddr_in _sender, bool isConnectAck = false);
	void SendAck(uint16 _ackId, sockaddr_in _target);
};

#endif // !I_NETWORK_BASE_H_DEFINED

