#include "ItemManager.h"
#include "../Gameplay/Scene/MainScene.h"
#include "../Gameplay/InventoryManager.h"

Scene* ItemManager::m_scene = nullptr;
Vector<EntityId> ItemManager::m_Items;
Vector<ItemData> ItemManager::m_ItemDefs;

void ItemManager::Init(Scene* _scene)
{
    m_scene = _scene;
    InitItemDefs();
}

void ItemManager::InitItemDefs()
{
    m_ItemDefs =
    {
        { ItemType::WaterBottle, "WaterBottle", "WaterBottleMaterial", 1.0f, 1.0f, 1.0f, 1.0f, 0.70f, 0.0f, true },
        { ItemType::Medic, "Medic", "MedicMaterial", 1.0f, 1.0f, 1.0f, 1.0f, 0.00f, 1.0f , false}
    };
}

ItemType ItemManager::PickRandomType()
{
    float total = 0.0f;
    for (auto& d : m_ItemDefs)
        total += d.bonusWeight;

    if (total <= 0.0f)
        return m_ItemDefs[0].type;

    float roll = (rand() / (float)RAND_MAX) * total;
    float cumul = 0.0f;
    for (auto& d : m_ItemDefs)
    {
        cumul += d.bonusWeight;
        if (roll < cumul)
            return d.type;
    }
    return m_ItemDefs[0].type;
}

EntityId ItemManager::SpawnItem(ItemType _type, float _x, float _y, float _z)
{
    ItemData* data = nullptr;
    for (auto& d : m_ItemDefs)
        if (d.type == _type) { data = &d; break; }
    if (!data) return 0;

    EntityId e = m_scene->world->CreateEntity();
    m_Items.push_back(e);
    TransformComponent& t = m_scene->world->AddComponent<TransformComponent>(e);
    MeshRenderer& mr = m_scene->world->AddComponent<MeshRenderer>(e);
    mr.geoId = RessourceManager::GetGeometryId(data->geometryName);
    mr.materialId = RessourceManager::GetMaterialId(data->materialName);
    ColliderComponent& col = m_scene->world->AddComponent<ColliderComponent>(e);
	col.SetBox(0.3f, 0.3f, 0.3f);
	RigidBodyComponent& rb = m_scene->world->AddComponent<RigidBodyComponent>(e);
	rb.SetMass(1.0f);
	rb.type = BodyType::Dynamic;
	rb.useGravity = true;
	rb.allowRotation = true;
	MotionComponent& motion = m_scene->world->AddComponent<MotionComponent>(e);
    t.local.SetPosition(XMFLOAT3(_x, _y + data->offsetY, _z));
    t.local.SetScale(XMFLOAT3(data->scaleX, data->scaleY, data->scaleZ));
    ItemCollectableComponent& collectable = m_scene->world->AddComponent<ItemCollectableComponent>(e);
    if(data->isFood)
		m_scene->world->AddComponent<ItemFoodComponent>(e);
    if(data->type == ItemType::Medic)
		m_scene->world->AddComponent<ItemMedicComponent>(e);

    return e;
}

void ItemManager::RegisterItem(EntityId _item)
{
	if (_item == 0) return;
	m_Items.push_back(_item);
}

void ItemManager::SpawnItems(Vector<Vector<char>>& _grid, int _count, int _xMin, int _xMax, int _yMin, int _yMax, float _cellSize, int _levelNb, int _nbPlayer)
{
    _count += ((_levelNb - 1) * (_nbPlayer * 2));
	std::cout << "Spawning " << _count << " items\n";

    std::vector<std::pair<int, int>> emptyCells;
    for (int x = 0; x < (int)_grid.size(); x++)
        for (int y = 0; y < (int)_grid[0].size(); y++)
            if (_grid[x][y] == ' ' &&
                !(x >= _xMin && x <= _xMax && y >= _yMin && y <= _yMax))
                emptyCells.push_back({ x, y });

    for (int i = (int)emptyCells.size() - 1; i > 0; i--)
        std::swap(emptyCells[i], emptyCells[rand() % (i + 1)]);

    std::vector<ItemType> toSpawn;
    for (auto& d : m_ItemDefs)
    {
        int guaranteed = (int)(_count * d.guaranteedPercent);
        for (int i = 0; i < guaranteed; i++)
            toSpawn.push_back(d.type);
    }

    int remaining = _count - (int)toSpawn.size();
    for (int i = 0; i < remaining; i++)
        toSpawn.push_back(PickRandomType());

    for (int i = (int)toSpawn.size() - 1; i > 0; i--)
        std::swap(toSpawn[i], toSpawn[rand() % (i + 1)]);

    float offsetX = _grid.size() * 0.5f * _cellSize - _cellSize / 2.0f;
    float offsetY = _grid[0].size() * 0.5f * _cellSize - _cellSize / 2.0f;

    for (int placed = 0; placed < (int)toSpawn.size() && placed < (int)emptyCells.size(); placed++)
    {
        float wx = emptyCells[placed].first * _cellSize - offsetX;
        float wz = emptyCells[placed].second * _cellSize - offsetY;
        SpawnItem(toSpawn[placed], wx, 0.0f, wz);
        _grid[emptyCells[placed].first][emptyCells[placed].second] = 'I';
    }
}

void ItemManager::DestroyItems()
{
    EntityId* inventoryItems = InventoryManager::GetInventory();

    for (EntityId item : m_Items)
    {
        bool inInventory = false;
        for (int i = 0; i < InventoryManager::GetItemCount(); i++)
        {
            if (inventoryItems[i] == item)
            {
                inInventory = true;
                break;
            }
        }
        if (!inInventory)
            m_scene->world->DestroyEntity(item);
	}
	m_Items.clear();
}