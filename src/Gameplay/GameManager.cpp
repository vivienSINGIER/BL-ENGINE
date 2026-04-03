#include "GameManager.h"

int       GameManager::m_quota = 0;
int       GameManager::m_collected = 0;
int       GameManager::m_levelNb = 0;
int       GameManager::m_nbPlayer = 0;
float     GameManager::m_lobbyRadius = 6.0f;
GameState GameManager::m_gameState = GameState::PLAYING;
bool      GameManager::m_validated = false;
int 	 GameManager::m_currentLevel = 1;

void GameManager::Init(int _nbPlayer)
{
	m_nbPlayer = _nbPlayer;
	m_levelNb = m_currentLevel;
	m_quota = GetQuotaForLevel();
	m_collected = 0;
	m_gameState = GameState::PLAYING;
	m_validated = false;
}

void GameManager::NextLevel()
{
	if(m_validated)
	{
		m_currentLevel++;
		m_quota = GetQuotaForLevel();
		m_collected = 0;
		m_gameState = GameState::PLAYING;
		m_validated = false;
		Reset();
	}
	else
	{
		m_gameState = GameState::LOSE;
	}
}

void GameManager::Reset()
{
	Init(m_nbPlayer);
}

void GameManager::CollectFood()
{
	std::cout << "GameState: " << (int)m_gameState << " collected: " << m_collected << "/" << m_quota << "\n";
	if(m_gameState != GameState::PLAYING)
		return;
	std::cout << "Food collected!" << std::endl;
	m_collected++;
	std::cout << "Collected: " << m_collected << "/" << m_quota << std::endl;
}

bool GameManager::TryValidateQuota(XMFLOAT3 _playerPos)
{
	if(m_gameState != GameState::PLAYING)
		return false;
	if(m_validated)
		return true;
	if(m_collected < m_quota)
		return false;

	float dist = sqrtf(_playerPos.x * _playerPos.x + _playerPos.y * _playerPos.y + _playerPos.z * _playerPos.z);
	if(dist <= m_lobbyRadius)
	{
		m_validated = true;
		return true;
	}
	else
	{
		return false;
	}
}

int GameManager::GetQuotaForLevel()
{
	return 3 + (m_levelNb - 1) * m_nbPlayer;
}

