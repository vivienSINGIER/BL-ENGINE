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
	InputUpdate
};

enum class InputState : uint8_t
{
	Down = 0,
	Pressed = 1,
	Up = 2
};

struct PacketHeader
{
	uint32_t tick;
	PacketType type;
	EntityId entityId;
};

struct PacketBody
{
	PacketHeader header;
	ComponentMask componentMask;
	uint8_t data[512];
	uint32_t dataSize;
};

struct InputEntry
{
	uint8_t keyCode;
	InputState state;
};

struct InputPacket
{
	PacketHeader header;
	uint32_t inputCount;
	InputEntry inputs[32];
};

#endif // !PACKET_HPP_DEFINED
