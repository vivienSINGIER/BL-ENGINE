#ifndef CONTACT_MANAGER_HPP_DEFINED
#define CONTACT_MANAGER_HPP_DEFINED

#include "define.h"

struct ContactPoint
{
    XMFLOAT3 position = { 0.0f, 0.0f, 0.0f };
};

struct Contact
{
    EntityId a = -1;
    EntityId b = -1;

    XMFLOAT3 normal = { 0.0f, 0.0f, 0.0f }; // A -> B
    float penetration = 0.0f;

    int pointCount = 0;
    ContactPoint points[4];
};

struct ContactManager
{
    std::vector<Contact> contacts;

    void Clear()
    {
        contacts.clear();
    }

    void AddContact(const Contact& c)
    {
        contacts.push_back(c);
    }
};

#endif // !CONTACT_MANAGER_HPP_DEFINED

