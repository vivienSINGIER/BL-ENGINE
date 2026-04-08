#include "ItemManager.h"
#include "../Gameplay/Scene/MainScene.h"

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
        { ItemType::WaterBottle, "WaterBottle", "WaterBottleMaterial", 1.0f, 1.0f, 1.0f, 1.0f, 5 , true},
        { ItemType::Medic, "Medic", "MedicMaterial", 1.0f , 1.0f, 1.0f, 1.0f , 5 , false}
    };
}

ItemType ItemManager::PickRandomType()
{
    int total = 0;
    for (auto& d : m_ItemDefs)
        total += d.weight;

    int roll = rand() % total;

    int cumul = 0;
    for (auto& d : m_ItemDefs)
    {
        cumul += d.weight;
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
	rb.allowRotation = false;
	MotionComponent& motion = m_scene->world->AddComponent<MotionComponent>(e);
    t.local.SetPosition(XMFLOAT3(_x, _y + data->offsetY, _z));
    t.local.SetScale(XMFLOAT3(data->scaleX, data->scaleY, data->scaleZ));
    if(data->collectible == true)
        ItemCollectableComponent& collectable = m_scene->world->AddComponent<ItemCollectableComponent>(e);

    return e;
}

void ItemManager::SpawnItems(Vector<Vector<char>>& _grid, int _count, int _xMin, int _xMax, int _yMin, int _yMax, float _cellSize, int _levelNb, int _nbPlayer)
{
    _count += (_levelNb - 1) * _nbPlayer;

    std::vector<std::pair<int, int>> emptyCells;
    for (int x = 0; x < (int)_grid.size(); x++)
        for (int y = 0; y < (int)_grid[0].size(); y++)
            if (_grid[x][y] == ' ' && !(x >= _xMin && x <= _xMax && y >= _yMin && y <= _yMax))
                emptyCells.push_back({ x, y });

    for (int i = emptyCells.size() - 1; i > 0; i--)
        std::swap(emptyCells[i], emptyCells[rand() % (i + 1)]);

    float offsetX = _grid.size() * 0.5f * _cellSize - _cellSize / 2;
    float offsetY = _grid[0].size() * 0.5f * _cellSize - _cellSize / 2;

    int placed = 0;
    for (const auto& cell : emptyCells)
    {
        if (placed >= _count) break;

        ItemType type = PickRandomType();

        float wx = cell.first * _cellSize - offsetX;
        float wz = cell.second * _cellSize - offsetY;
        SpawnItem(type, wx, 0.0f, wz);

        _grid[cell.first][cell.second] = 'I';
        placed++;
    }
}

void ItemManager::DestroyItems()
{
    for (EntityId e : m_Items)
        m_scene->world->DestroyEntity(e);
    m_Items.clear();
}