#ifndef ITEM_MANAGER_H_DEFINED
#define ITEM_MANAGER_H_DEFINED

#include "../Engine/Engine.h"

enum class ItemType
{
    WaterBottle,
    Medic
};

struct ItemData
{
    ItemType type;
    std::string geometryName;
    std::string materialName;
    float scaleX, scaleY, scaleZ;
    float offsetY; // hauteur au sol
	int weight; // poids de l'item pour le spawn
};

class ItemManager
{
public:
    static void Init(Scene* _scene);
    static void SpawnItems(Vector<Vector<char>>& _grid, int _count, int _xMin, int _xMax, int _yMin, int _yMax, float _cellSize, int _levelNb, int _nbPlayer);
    static void DestroyItems();

	static ItemType PickRandomType();
    static EntityId SpawnItem(ItemType _type, float _x, float _y, float _z);

private:
    static Scene* m_scene;
    static Vector<EntityId> m_Items;
    static Vector<ItemData> m_ItemDefs;
    static void InitItemDefs();
};

#endif