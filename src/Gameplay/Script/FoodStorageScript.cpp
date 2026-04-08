#include "FoodStorageScript.h"
#include "../Gameplay/GameManager.h"
#include "../Gameplay/Scene/MainScene.h"
#include "../Gameplay/InventoryManager.h"

void FoodStorageScript::OnTrigger(EntityId _otherId)
{
	Scene* s = SceneManager::GetSceneWithId(sceneId);
	if (s->world->HasComponent<ItemCollectableComponent>(_otherId) && s->world->HasComponent<ItemFoodComponent>(_otherId) && s->world->IsActive(_otherId))
	{
		TransformComponent& otherT = s->world->GetComponent<TransformComponent>(_otherId);
		InventoryManager::ThrowSelectedItem(0.0f);
		GameManager::CollectFood();
		s->world->SetInactive(_otherId);
		otherT.local.SetPosition(XMFLOAT3(0.0f, -10.0f, 0.0f));
	}
}
