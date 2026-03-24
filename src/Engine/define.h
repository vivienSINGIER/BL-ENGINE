#ifndef ENGINE_DEFINE_H_DEFINED
#define ENGINE_DEFINE_H_DEFINED

#include "Core/define.h"

#include <WS2tcpip.h>
#include <winsock2.h>

#include <bitset>
#include <cassert>

#define MAX_COMPONENTS 256

using ComponentMask = std::bitset<MAX_COMPONENTS>;
using ComponentId = uint32_t;
using EntityId = uint64_t;

enum NetworkFlag : uint8
{
    NONE = 0,
    SERVER = 1 << 0,
    CLIENT = 1 << 1,
};

enum InputState : uint8
{
    NONE_STATE            = 0,
    DOWN_STATE      = 1 << 0,
    PRESSED_STATE   = 1 << 1,
    UP_STATE        = 1 << 2
};

#endif
