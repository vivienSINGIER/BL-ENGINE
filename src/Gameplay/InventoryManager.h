#ifndef INVENTORY_MANAGER_H_DEFINED
#define INVENTORY_MANAGER_H_DEFINED

#include "../Engine/Engine.h"

class InventoryManager
{
public:
	static void Init(EntityId _ownerId);
	static void AddItem(EntityId _item);
	static void RemoveItem(EntityId _item);
	static EntityId TakeItem(int _index);
	static void Update(float dt);
	static void ThrowSelectedItem(float _strength);
	static bool FullInventory() { return m_itemId[0] != 0 && m_itemId[1] != 0 && m_itemId[2] != 0; }
	static void ResetInventory();
	static EntityId* GetInventory() { return m_itemId; }
	static int GetItemCount() { return 3; }

private:
	static EntityId m_ownerId;
	static EntityId m_itemId[3];
	static EntityId m_selectedItem;
};
#endif // !INVENTORY_MANAGER_H_DEFINED

