#ifndef GAS_MANAGER_H_DEFINED
#define GAS_MANAGER_H_DEFINED

#include "../Engine/Engine.h"
#include "../Gameplay/PlayerHealth.hpp"

class GasManager
{
public:
	static void SetGasWalls(const float _startX[4], const float _targetX[4]);
	static void Update(float _dt, float _gasTimer, float _gasDuration, Scene* _scene, EntityId _players[], int _nbPlayers);
    static void SetBaseDamagePerSecond(float _dps) { m_baseDPS = _dps; }

private:
    static float CurrentWallPos(int _i, float _t);

    static bool IsBehindWall(int _wallIndex, float _wallCurrentPos, const XMFLOAT3& _pos);

    static float m_startX[4];
    static float m_targetX[4];
    static float m_baseDPS;
};
#endif // !GAS_MANAGER_H_DEFINED


