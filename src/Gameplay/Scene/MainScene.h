#ifndef MAIN_SCENE_H_DEFINED
#define MAIN_SCENE_H_DEFINED

#include "../Engine/Engine.h"

class MainScene : public Scene
{
public:
	void OnInit() override;
	void OnUpdate(float _dt) override;
	void OnStart() override;
	void OnEnd() override;

private:
	EntityId m_camera;
	EntityId m_test;
};

#endif // !MAIN_SCENE_H_DEFINED
