#include "GasManager.h"
#include <iostream>

float GasManager::m_startX[4] = {};
float GasManager::m_targetX[4] = {};
float GasManager::m_baseDPS = 25.0f; 

void GasManager::SetGasWalls(const float _startX[4], const float _targetX[4])
{
    for (int i = 0; i < 4; i++)
    {
        m_startX[i] = _startX[i];
        m_targetX[i] = _targetX[i];
    }
}

float GasManager::CurrentWallPos(int _i, float _t)
{
    return m_startX[_i] + (m_targetX[_i] - m_startX[_i]) * _t;
}

bool GasManager::IsBehindWall(int _wallIndex, float _wallCurrentPos, const XMFLOAT3& _pos)
{
    switch (_wallIndex)
    {
    case 0: return _pos.x < _wallCurrentPos;  
    case 1: return _pos.x > _wallCurrentPos;  
    case 2: return _pos.z < _wallCurrentPos;
    case 3: return _pos.z > _wallCurrentPos;
    default: return false;
    }
}

void GasManager::Update(float _dt, float _gasTimer, float _gasDuration, Scene* _scene, EntityId _players[], int _nbPlayers)
{
    if (_gasDuration <= 0.0f) return;

    float t = _gasTimer / _gasDuration;
    if (t > 1.0f) t = 1.0f;

    float speedFactor = 1.0f + t;

    for (int p = 0; p < _nbPlayers; p++)
    {
        EntityId playerId = _players[p];
        if (playerId == 0) continue;
        if (!_scene->world->HasComponent<PlayerHealthComponent>(playerId)) continue;
        if (!_scene->world->HasComponent<TransformComponent>(playerId))   continue;

        TransformComponent& tr = _scene->world->GetComponent<TransformComponent>(playerId);
        PlayerHealthComponent& hp = _scene->world->GetComponent<PlayerHealthComponent>(playerId);

        if (hp.isDead) continue;

        XMFLOAT3 pos = tr.world.GetPosition();

        bool inGas = false;
        for (int i = 0; i < 4; i++)
        {
            float wallPos = CurrentWallPos(i, t);
            if (IsBehindWall(i, wallPos, pos))
            {
                inGas = true;
                break;
            }
        }

        if (inGas)
        {
            float dmg = m_baseDPS * speedFactor * _dt;
            hp.TakeDamage(dmg);
            std::cout << "[Gas] Player " << p << " HP: " << hp.health  << " (dmg this frame: " << dmg << ")\n";
        }
    }
}