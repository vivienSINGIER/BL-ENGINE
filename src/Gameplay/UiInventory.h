#ifndef UI_INVENTORY_H_DEFINED
#define UI_INVENTORY_H_DEFINED

#include "../Engine/Engine.h"
#include "../Gameplay/Scene/MainScene.h"

class UiInventory
{
public:
	static void Init(Scene* _scene);
	static void AddItemToInventory(int _index, ItemType _type);
	static void RemoveItemFromInventory(int _index);

private:
	static void CreateBottle(int _index);
	static void CreateMedic(int _index);

	static Scene* m_scene;
	static EntityId m_Item[3];

	static XMFLOAT2 m_posIndex1;
	static XMFLOAT2 m_posIndex2;
	static XMFLOAT2 m_posIndex3;
};
#endif // !UI_INVENTORY_H_DEFINED

