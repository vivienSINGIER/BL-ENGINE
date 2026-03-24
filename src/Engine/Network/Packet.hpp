#ifndef PACKET_HPP_DEFINED
#define PACKET_HPP_DEFINED

#include <cstdint>
#include "define.h"

#define MAGIC_WORD 0b1010101111001101

enum class PacketType : uint8
{
	Connect,
	ConnectAck,
	Ack,

	AddScene,
	SetScene,
	
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
	uint16		magicWord = MAGIC_WORD;
	uint32      tick;
	uint16      ackId;
	PacketType  type;
	EntityId    entityId;
};

struct ConnectPacket
{
	PacketHeader header;
	sockaddr_in addr;
};

struct AddScenePacket
{
	PacketHeader header;
	uint32 sceneId;
	uint8 nameSize;
	char name[25];
};

struct SetScenePacket
{
	PacketHeader header;
	uint32 sceneId;
};

struct ComponentEntry
{
	uint32	ComponentId;
	uint32	size;
	Byte	data[1024];
};

struct StatePacket
{
	PacketHeader    header;
	ComponentMask   componentMask;
	uint32			dataSize;
	uint8			componentCount;
	ComponentEntry	components[10];
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
		ConnectPacket   connect;
		AddScenePacket  addScene;
		SetScenePacket  setScene;
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
		case PacketType::Connect:			return sizeof(ConnectPacket);
		case PacketType::ConnectAck:		return sizeof(ConnectPacket);
		case PacketType::AddScene:			return sizeof(PacketHeader) + sizeof(uint32) + sizeof(uint8) + sizeof(char) * addScene.nameSize;
		case PacketType::SetScene:			return sizeof(SetScenePacket);
		case PacketType::AddComponent:		return sizeof(PacketHeader) + sizeof(ComponentMask) + state.dataSize;
		case PacketType::RemoveComponent:   return sizeof(PacketHeader) + sizeof(ComponentMask) + state.dataSize;
		case PacketType::InputUpdate:       return sizeof(PacketHeader) + sizeof(uint32) + sizeof(InputEntry) * input.inputCount;
		case PacketType::Chat:              return sizeof(PacketHeader) + sizeof(uint32) + chat.messageLength;
		default:                            return sizeof(PacketHeader);
		}
	}

	char* Data() { return reinterpret_cast<char*>(this); }
};

#endif // !PACKET_HPP_DEFINED
