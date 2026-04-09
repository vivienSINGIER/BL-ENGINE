#include "UiInventory.h"

XMFLOAT2 UiInventory::m_posIndex1 = XMFLOAT2(-200.0f, -400.0f);
XMFLOAT2 UiInventory::m_posIndex2 = XMFLOAT2(0.0f, -400.0f);
XMFLOAT2 UiInventory::m_posIndex3 = XMFLOAT2(200.0f, -400.0f);
Scene*	 UiInventory::m_scene = nullptr;
EntityId UiInventory::m_Item[3] = { 0, 0, 0 };

void UiInventory::Init(Scene* _scene)
{
	m_scene = _scene;
}

void UiInventory::AddItemToInventory(int _index, ItemType _type)
{
	if(_type == ItemType::Food)
		CreateBottle(_index);
	if(_type == ItemType::Medic)
		CreateMedic(_index);
}

void UiInventory::RemoveItemFromInventory(int _index)
{
	m_scene->world->DestroyEntity(m_Item[_index]);
}

void UiInventory::CreateBottle(int _index)
{
	m_Item[_index] = m_scene->world->CreateEntity();
	UiImageComponent& img = m_scene->world->AddComponent<UiImageComponent>(m_Item[_index]);

	uint32 spriteId = RessourceManager::GetSpriteId("Sprite");
	uint32 matId = RessourceManager::GetUiMaterialId("WaterBottleUiMat");

	img.spriteId = spriteId;
	img.materialId = matId;
	if (_index == 0)
	{
		img.transform.SetPosition(m_posIndex1);
	}
	else if (_index == 1)
	{
		img.transform.SetPosition(m_posIndex2);
	}
	else if (_index == 2)
	{
		img.transform.SetPosition(m_posIndex3);
	}
}

void UiInventory::CreateMedic(int _index)
{
	m_Item[_index] = m_scene->world->CreateEntity();
	UiImageComponent& img = m_scene->world->AddComponent<UiImageComponent>(m_Item[_index]);
	img.spriteId = RessourceManager::GetSpriteId("Sprite");
	img.materialId = RessourceManager::GetUiMaterialId("MedicUiMat");
	if (_index == 0)
	{
		img.transform.SetPosition(m_posIndex1);
	}
	else if (_index == 1)
	{
		img.transform.SetPosition(m_posIndex2);
	}
	else if (_index == 2)
	{
		img.transform.SetPosition(m_posIndex3);
	}
}


