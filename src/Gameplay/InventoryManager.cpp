#include "InventoryManager.h"
#include "../Gameplay/LevelManager.h"
#include "../Gameplay/UiInventory.h"

EntityId InventoryManager::m_itemId[3] = { 0, 0, 0 };
EntityId InventoryManager::m_selectedItem = 0;
EntityId InventoryManager::m_ownerId = 0;

void InventoryManager::Init(EntityId _ownerId)
{
	m_ownerId = _ownerId;
	UiInventory::Init(SceneManager::GetSceneWithName("MainScene"));
}

void InventoryManager::AddItem(EntityId _item)
{
	ItemCollectableComponent& itemComp = SceneManager::GetCurrentScene()->world->GetComponent<ItemCollectableComponent>(_item);
	for (int i = 0; i < 3; i++)
	{
		if (m_itemId[i] == 0)
		{
			UiInventory::AddItemToInventory(i, itemComp.type);
			m_itemId[i] = _item;
			return;
		}
	}
}

void InventoryManager::RemoveItem(EntityId _item)
{
	for (int i = 0; i < 3; i++)
	{
		if (m_itemId[i] == _item)
		{
			UiInventory::RemoveItemFromInventory(i);
			m_itemId[i] = 0;
			if(m_selectedItem == _item)
				m_selectedItem = 0;
			return;
		}
	}
}

EntityId InventoryManager::TakeItem(int _index)
{
    if (_index < 0 || _index >= 3)
        return 0;

    Scene* scene = SceneManager::GetCurrentScene();

    if (m_itemId[_index] == m_selectedItem && m_selectedItem != 0)
    {
        scene->world->SetInactive(m_selectedItem);
        m_selectedItem = 0;
        return 0;
    }

    if (m_selectedItem != 0)
    {
        scene->world->SetInactive(m_selectedItem);
        m_selectedItem = 0;
    }

    if (m_itemId[_index] == 0)
        return 0;

    m_selectedItem = m_itemId[_index];
    return m_selectedItem;
}

void InventoryManager::Update(float dt)
{
	Scene* scene = SceneManager::GetCurrentScene();
	if(m_selectedItem != 0)
	{
		TransformComponent& t = scene->world->GetComponent<TransformComponent>(m_selectedItem);
		RigidBodyComponent& rb = scene->world->GetComponent<RigidBodyComponent>(m_selectedItem);
		t.SetParent(m_ownerId);
		rb.useGravity = false;
		t.local.SetPosition(XMFLOAT3(0.0f, 0.0f, 1.5f));
		scene->world->SetActive(m_selectedItem);
	}
}

void InventoryManager::ThrowSelectedItem(float _strength)
{
    Scene* scene = SceneManager::GetCurrentScene();
    if (m_selectedItem == 0) return;

    EntityId itemToThrow = m_selectedItem;
    RemoveItem(itemToThrow);

    TransformComponent& t = scene->world->GetComponent<TransformComponent>(itemToThrow);
    TransformComponent& playerT = scene->world->GetComponent<TransformComponent>(m_ownerId);
    RigidBodyComponent& rb = scene->world->GetComponent<RigidBodyComponent>(itemToThrow);
    MotionComponent& motion = scene->world->GetComponent<MotionComponent>(itemToThrow);

    XMFLOAT3 forward = playerT.world.GetForward();
    XMFLOAT3 pos = playerT.world.GetPosition();
    pos.x += forward.x * 2.0f;
    pos.y += forward.y * 2.0f;
    pos.z += forward.z * 2.0f;
    t.RemoveParent();
    t.local.SetPosition(pos);
    rb.useGravity = true;

    motion.linearVelocity = { 0.0f, 0.0f, 0.0f };
    motion.angularVelocity = { 0.0f, 0.0f, 0.0f };
    motion.force = { 0.0f, 0.0f, 0.0f };
    motion.torque = { 0.0f, 0.0f, 0.0f };
    motion.isSleeping = false;
    motion.sleepTimer = 0.0f;

    motion.AddLinearImpulse(XMFLOAT3(forward.x * _strength, forward.y * _strength, forward.z * _strength));
}
