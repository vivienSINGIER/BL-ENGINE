#ifndef PACKET_HPP_DEFINED
#define PACKET_HPP_DEFINED
#include <cstdint>
#include "Engine.h"

enum class PacketType : uint8
{
	Connect,
	ConnectAck,
	Ack,
	
	Spawn,
	Delete,
	
	AddComponent,
	RemoveComponent,
	Update,
	
	InputUpdate,
	Chat,
};

struct PacketHeader
{
	uint32      tick;
	uint16      ackId;
	PacketType  type;
	EntityId    entityId;
};

struct ComponentEntry
{
	uint32 ComponentId;
	uint32 size;
	Byte data[1024];
};

struct StatePacket
{
	PacketHeader    header;
	ComponentMask   componentMask;
	uint32 dataSize;
	uint8 componentCount;
	ComponentEntry components[10];
};

struct InputEntry
{
	uint8       keyCode;
	InputState  state;
};

struct InputPacket
{
	PacketHeader    header;
	uint32          inputCount;
	InputEntry      inputs[32];
};

struct ChatPacket
{
	PacketHeader    header;
	uint32          messageLength;
	char            message[256];
};

struct Packet
{
	union
	{
		PacketHeader    header;
		StatePacket     state;
		InputPacket     input;
		ChatPacket      chat;
	};

	Packet() { memset(this, 0, sizeof(Packet)); }
	
	uint32 Size() const
	{
		switch (header.type)
		{
		case PacketType::Spawn:
		case PacketType::Update:
		case PacketType::AddComponent:
		case PacketType::RemoveComponent:   return sizeof(PacketHeader) + sizeof(ComponentMask) + sizeof(uint32) + state.dataSize;
		case PacketType::InputUpdate:       return sizeof(PacketHeader) + sizeof(uint32) + sizeof(InputEntry) * input.inputCount;
		case PacketType::Chat:              return sizeof(PacketHeader) + sizeof(uint32) + chat.messageLength;
		default:                            return sizeof(PacketHeader);
		}
	}

	char* Data() { return reinterpret_cast<char*>(this); }
};

#endif // !PACKET_HPP_DEFINED
