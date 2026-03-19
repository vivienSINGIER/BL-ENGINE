#ifndef PACKET_HPP_DEFINED
#define PACKET_HPP_DEFINED
#include <cstdint>
#include "Engine.h"

enum class PacketType
{
	Update,
	Spawn,
	AddComponent,
	RemoveComponent,
	Delete,
	InputUpdate,
	Connect,
	ConnectAck,
	Chat,
	Ack
};


struct PacketHeader
{
	uint32_t tick;
	PacketType type;
	EntityId entityId;
	uint16 ackId;
};

struct AckPacket
{
	PacketHeader header;
	uint16 ackId;
};

struct Packet
{
	PacketHeader header;
	ComponentMask componentMask;
	uint8 data[512];
	uint32 dataSize;
};

struct InputEntry
{
	uint8 keyCode;
	InputState state;
};

struct InputPacket
{
	PacketHeader header;
	uint32 inputCount;
	InputEntry inputs[32];
};

#endif // !PACKET_HPP_DEFINED
