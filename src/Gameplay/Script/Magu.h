#ifndef MAGU_H_DEFINED
#define MAGU_H_DEFINED

#include "../Engine/Engine.h"

enum class MaguState
{
	WANDERING,
	STEALING,
	CARRYING,
	DROPING
};

struct Magu : public IScript
{
public:
	void Awake() override;
	void Update(float _dt) override;

	void SetSizeLabyrinthe(float _size);
	float detectionRadius = 5.0f;
	float playerSafeRadius = 10.0f;
	float moveSpeed = 2.0f;
	float carryDuration = 5.0f;

	void SetPlayer(int playerIndex, EntityId playerEntity) { if (playerIndex >= 0 && playerIndex < 4) m_player[playerIndex] = playerEntity; }

private:
	MaguState m_state = MaguState::WANDERING;

	XMFLOAT3 m_targetPosition;
	float m_wanderTimer = 0.0f;
	float m_wanderInterval = 3.0f;

	int m_targetStickIndex = -1;
	float m_carryTimer = 0.0f;
	float m_labyrintheSize = 0.0f;

	EntityId m_player[4];

	void PickNewWanderTarget();
	void MoveTowards(XMFLOAT3 _dest, float _dt);
	bool ReachedTagert(XMFLOAT3 _dest, float _threshold = 0.5f);
	XMFLOAT3 GetMyPos();
	bool IsPlayerFarEnough(float _x, float _z);

};
#endif // !MAGU_H_DEFINED
