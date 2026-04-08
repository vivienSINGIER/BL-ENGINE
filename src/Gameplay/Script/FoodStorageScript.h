#ifndef FOOD_STORAGE_SCRIPT_H_DEFINED
#define FOOD_STORAGE_SCRIPT_H_DEFINED

#include "../Engine/Engine.h"

struct FoodStorageScript : public IScript
{
	void OnTrigger(EntityId _otherId) override;
};
#endif // !FOOD_STORAGE_SCRIPT_H_DEFINED


