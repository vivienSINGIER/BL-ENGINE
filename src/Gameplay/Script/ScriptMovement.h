#ifndef SCRIPT_MOVEMENT_H_DEFINED
#define SCRIPT_MOVEMENT_H_DEFINED

#include "../Engine/Engine.h"

class Movement
{
public:
	struct ScriptMovement : public IScript
	{
        void Update(float dt) override
        {
            TransformComponent& t = GetComponent<TransformComponent>();

            if (InputManager::IsKey(Z))
                t.local.Move(XMFLOAT3(0.0f, 0.0f, 1.0f * dt));
            if (InputManager::IsKey(S))
                t.local.Move(XMFLOAT3(0.0f, 0.0f, -1.0f * dt));
            if (InputManager::IsKey(Q))
                t.local.Move(XMFLOAT3(-1.0f * dt, 0.0f, 0.0f));
            if (InputManager::IsKey(D))
                t.local.Move(XMFLOAT3(1.0f * dt, 0.0f, 0.0f));
            if (InputManager::IsKey(SPACE))
                t.local.Move(XMFLOAT3(0.0f, 1.0f * dt, 0.0f));
            if (InputManager::IsKey(LCONTROL))
                t.local.Move(XMFLOAT3(0.0f, -1.0f * dt, 0.0f));
        }
	};
};

#endif // !SCRIPT_MOVEMENT_H_DEFINED
