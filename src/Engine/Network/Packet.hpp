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
	AddScript,
	RemoveComponent,
	RemoveScript,
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
	uint32      sceneId;
};

struct ConnectPacket
{
	PacketHeader header;
	sockaddr_in addr;
};

struct AddScenePacket
{
	PacketHeader header;
	uint8 nameSize;
	char name[25];
};

struct ComponentEntry
{
	uint32	ComponentId;
	uint32	size;
	Byte	data[1024];
};

struct AddComponentPacket
{
	PacketHeader header;
	uint32	ComponentId;
	uint32	size;
	Byte	data[1024];
};

struct AddScriptPacket
{
	PacketHeader header;
	uint32 ComponentId;
};

struct RemoveComponentPacket
{
	PacketHeader header;
	ComponentId cid;
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
		AddComponentPacket addComponent;
		AddScriptPacket  addScript;
		RemoveComponentPacket removeComponent;
		StatePacket     state;
		InputPacket     input;
		ChatPacket      chat;
	};

	Packet() { memset(this, 0, sizeof(Packet)); }
	
	uint32 Size() const
	{
		switch (header.type)
		{
		case PacketType::Spawn:				return sizeof(PacketHeader);
		case PacketType::Delete:			return sizeof(PacketHeader);
		case PacketType::Update:			
		case PacketType::Connect:			return sizeof(ConnectPacket);
		case PacketType::ConnectAck:		return sizeof(ConnectPacket);
		case PacketType::AddScene:			return sizeof(PacketHeader) + sizeof(uint8) + sizeof(char) * addScene.nameSize;
		case PacketType::SetScene:			return sizeof(PacketHeader);
		case PacketType::AddComponent:		return sizeof(PacketHeader) + sizeof(ComponentId) + sizeof(uint32) + addComponent.size;
		case PacketType::AddScript:			return sizeof(AddScriptPacket);
		case PacketType::RemoveComponent:   return sizeof(RemoveComponentPacket);
		case PacketType::InputUpdate:       return sizeof(PacketHeader) + sizeof(uint32) + sizeof(InputEntry) * input.inputCount;
		case PacketType::Chat:              return sizeof(PacketHeader) + sizeof(uint32) + chat.messageLength;
		default:                            return sizeof(PacketHeader);
		}
	}

	char* Data() { return reinterpret_cast<char*>(this); }
};

#endif // !PACKET_HPP_DEFINED
