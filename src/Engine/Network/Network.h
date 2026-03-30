#ifndef NETWORK_H_DEFINED
#define NETWORK_H_DEFINED

#include "define.h"

#include "Packet.hpp"
#include "Sockets.h"
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

struct ReceivedPacket
{
    Packet packet;
    sockaddr_in sender;
};

#endif