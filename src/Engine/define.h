#ifndef ENGINE_DEFINE_H_DEFINED
#define ENGINE_DEFINE_H_DEFINED

#include "Core/define.h"

#include <bitset>
#include <cassert>

#define MAX_COMPONENTS 256

using ComponentMask = std::bitset<MAX_COMPONENTS>;
using ComponentId = uint32_t;
using EntityId = uint64_t;

#endif
