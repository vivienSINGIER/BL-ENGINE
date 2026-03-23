#ifndef CONTACT_MANAGER_HPP_DEFINED
#define CONTACT_MANAGER_HPP_DEFINED

#include "define.h"

struct Contact
{
	EntityId a;
	EntityId b;
    
	XMFLOAT3 normal; //Direction A -> B
	XMFLOAT3 point;
	float penetration;
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

