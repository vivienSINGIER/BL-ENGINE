#ifndef PACKET_HPP_DEFINED
#define PACKET_HPP_DEFINED

#include <cstdint>
#include "define.h"

#define MAGIC_WORD 0b1010101111001101
#define MAX_INPUT_COUNT 32
#define MAX_COMPONENT_DATA_SIZE 1024
#define MAX_COMPONENT_UPDATE_COUNT 10
#define MAX_MESSAGE_LENGTH 256
#define MAX_SCENE_NAME 25

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
	
	KeyUpdate,
	MouseButtonUpdate,
	MousePosUpdate,
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
	uint32		clientId;
};

struct ConnectPacket
{
	PacketHeader header;
	sockaddr_in addr;
	uint32 cliendId;
};

struct AddScenePacket
{
	PacketHeader header;
	uint8 nameSize;
	char name[MAX_SCENE_NAME];
};

struct CreateEntityPacket
{
	PacketHeader header;
	ComponentMask componentMask;
};

struct ComponentEntry
{
	uint32	ComponentId;
	uint32	size;
	Byte	data[MAX_COMPONENT_DATA_SIZE];
};

struct AddComponentPacket
{
	PacketHeader header;
	uint32	ComponentId;
	uint32	size;
	Byte	data[MAX_COMPONENT_DATA_SIZE];
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

struct UpdatePacket
{
	PacketHeader    header;
	uint8			componentCount;
	ComponentEntry	components[MAX_COMPONENT_UPDATE_COUNT];
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
	InputEntry      inputs[MAX_INPUT_COUNT];
};

struct MousePosUpdate
{
	PacketHeader header;
	
	bool cursorLocked = false;
	bool cursorVisible = false;
	int32 x, y;
	float dx, dy;
};

struct ChatPacket
{
	PacketHeader    header;
	uint32          messageLength;
	char            message[MAX_MESSAGE_LENGTH];
};

struct Packet
{
	union
	{
		PacketHeader			header;
		ConnectPacket			connect;
		AddScenePacket			addScene;
		CreateEntityPacket		createEntity;
		AddComponentPacket		addComponent;
		AddScriptPacket			addScript;
		RemoveComponentPacket	removeComponent;
		UpdatePacket			update;
		InputPacket				input;
		MousePosUpdate			mouse;
		ChatPacket				chat;
	};

	Packet() { memset(this, 0, sizeof(Packet)); }
	
	uint32 Size() const
	{
		switch (header.type)
		{
		case PacketType::Spawn:				return sizeof(CreateEntityPacket);
		case PacketType::Delete:			return sizeof(PacketHeader);
		case PacketType::Update:			return sizeof(PacketHeader) + sizeof(uint8) + update.componentCount * sizeof(ComponentEntry);
		case PacketType::Connect:			return sizeof(ConnectPacket);
		case PacketType::ConnectAck:		return sizeof(ConnectPacket);
		case PacketType::AddScene:			return sizeof(PacketHeader) + sizeof(uint8) + sizeof(char) * addScene.nameSize;
		case PacketType::SetScene:			return sizeof(PacketHeader);
		case PacketType::AddComponent:		return sizeof(PacketHeader) + sizeof(ComponentId) + sizeof(uint32) + addComponent.size;
		case PacketType::AddScript:			return sizeof(AddScriptPacket);
		case PacketType::RemoveComponent:   return sizeof(RemoveComponentPacket);
		case PacketType::KeyUpdate:			return sizeof(PacketHeader) + sizeof(uint32) + sizeof(InputEntry) * input.inputCount;
		case PacketType::MouseButtonUpdate: return sizeof(PacketHeader) + sizeof(uint32) + sizeof(InputEntry) * input.inputCount;
		case PacketType::MousePosUpdate:	return sizeof(MousePosUpdate);
		case PacketType::Chat:              return sizeof(PacketHeader) + sizeof(uint32) + chat.messageLength;
		default:                            return sizeof(PacketHeader);
		}
	}

	char* Data() { return reinterpret_cast<char*>(this); }
};

#endif // !PACKET_HPP_DEFINED
