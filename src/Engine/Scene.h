#ifndef SCENE_H_DEFINED
#define SCENE_H_DEFINED

#include "define.h"
#include "ECS/World.h"

class Scene
{
public:
	String GetName() { return m_name; }

	virtual ~Scene() {}

	World world;
	
protected:
	virtual void OnInit() {}
	virtual void OnUpdate(float _dt) {}
	virtual void OnStart() {}
	virtual void OnEnd() {}

private:
	String m_name;

	void Init(String const& _name);
	void Update(float _dt);

	friend class EngineManager;
	friend class SceneManager; 
};

#endif