#ifndef NETWORK_COMPONENT_HPP_DEFINED
#define NETWORK_COMPONENT_HPP_DEFINED

#include "Engine.h"

struct NetworkComponent
{
	ComponentMask mask = 0;
	bool dirty = false;
};

#endif // !NETWORK_COMPONENT_DEFINED
