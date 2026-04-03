#ifndef GAME_MANAGER_H_DEFINED
#define GAME_MANAGER_H_DEFINED

#include "../Engine/Engine.h"

enum class GameState
{
	PLAYING,
	WIN,
	LOSE
};
class GameManager
{
public:
	static void Init(int _nbPlayer);
	static void NextLevel();
	static void Reset();

	static void CollectFood();

	static bool TryValidateQuota(XMFLOAT3 _playerPos);

	static int GetQuota() { return m_quota; }
	static int GetCollected() { return m_collected; }
	static GameState GetGameState() { return m_gameState; }

private:
	static int m_quota;
	static int m_collected;
	static int m_levelNb;
	static int m_currentLevel;
	static int m_nbPlayer;
	static GameState m_gameState;
	static bool m_validated;
	static float m_lobbyRadius;

	static int GetQuotaForLevel();
};
#endif // !GAME_MANAGER_H_DEFINED

