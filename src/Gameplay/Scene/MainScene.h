#ifndef MAIN_SCENE_H_DEFINED
#define MAIN_SCENE_H_DEFINED

#include "../Engine/Engine.h"

struct ItemCollectableComponent {};
struct ItemFoodComponent {};
struct ItemMedicComponent {};

class MainScene : public Scene
{
public:
	void OnInit() override;
	void OnUpdate(float _dt) override;
	void OnStart() override;
	void OnEnd() override;
	void LoadRessources() override;

private:
	EntityId m_camera;
	EntityId m_playerCube;
	EntityId m_test;
	EntityId m_laby;
	EntityId m_light;

	float m_pitch;

	EntityId m_player[4];

	XMFLOAT3 m_lightPos;
	float m_lightTravelDistance;
	float m_lightPosXStart;

	bool m_opened;

	bool m_isDay;
	bool m_isNight;
	bool m_gasStarted;
	float m_timer;
	float m_dayDuration;
	float m_nightDuration;
	XMFLOAT3 m_dayColorStart;
	
	bool m_started;
	
	void SetStarted(bool _started) { m_started = _started; }
	void PreserveInventory();
	void ReRegisterInventoryItem();
	int m_levelSize;

	bool m_skipNextFrame = false;
};

#endif // !MAIN_SCENE_H_DEFINED
