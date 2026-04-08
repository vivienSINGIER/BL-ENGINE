#include "ScriptMovement.h"
#include "../Gameplay/GlowStick.h"
#include "../Gameplay/Scene/MainScene.h"
#include "../Gameplay/GameManager.h"

void Movement::ScriptMovement::Awake()
{
    m_yaw = 0.0f;
	m_pitch = 0.0f;
	InputManager::LockMouseCursor();
	InputManager::HideMouseCursor();
	m_cursorLocked = InputManager::IsMouseCursorLocked();

	for(int i = 0; i < 3; i++)
	{
		m_glowStick[i] = SceneManager::GetSceneWithId(sceneId)->world->CreateEntity();
		SceneManager::GetSceneWithId(sceneId)->world->AddScript<GlowStick>(m_glowStick[i]);
	}
}

void Movement::ScriptMovement::Update(float dt)
{
	m_cursorLocked = InputManager::IsMouseCursorLocked();
    TransformComponent& t = GetComponent<TransformComponent>();
	MotionComponent& motion = GetComponent<MotionComponent>();

	float mouseSensitivity = 0.01f;
	XMFLOAT2 mouseDelta = InputManager::GetMouseDelta();
	m_yaw += mouseDelta.x * mouseSensitivity;

	if (m_cursorLocked)
	{
		t.local.SetYPR(XMFLOAT3(m_yaw, m_pitch, 0.0f));
	}

	float moveSpeed = 30.0f;

	XMFLOAT3 forward = t.local.GetForward();
	XMFLOAT3 right = t.local.GetRight();

	if (InputManager::IsKey(Z))
		motion.AddLinearImpulse(XMFLOAT3(forward.x * moveSpeed * dt, 0.0f, forward.z * moveSpeed * dt));
	if (InputManager::IsKey(S))
		motion.AddLinearImpulse(XMFLOAT3(-forward.x * moveSpeed * dt, 0.0f, -forward.z * moveSpeed * dt));
	if (InputManager::IsKey(Q))
		motion.AddLinearImpulse(XMFLOAT3(-right.x * moveSpeed * dt, 0.0f, -right.z * moveSpeed * dt));
	if (InputManager::IsKey(D))
		motion.AddLinearImpulse(XMFLOAT3(right.x * moveSpeed * dt, 0.0f, right.z * moveSpeed * dt));
	if(InputManager::IsKey(SPACE))
		motion.AddLinearImpulse(XMFLOAT3(0.0f, moveSpeed * dt, 0.0f));
	if(InputManager::IsKeyDown(ESCAPE))
	{
		if(m_cursorLocked)
		{
			InputManager::UnlockMouseCursor();
			InputManager::ShowMouseCursor();
		}
		else
		{
			InputManager::LockMouseCursor();
			InputManager::HideMouseCursor();
		}
	}

	if(InputManager::IsKeyDown(G))
	{
		for (int i = 0; i < 3; i++)
		{
			if (SceneManager::GetSceneWithId(sceneId)->world->IsActive(m_glowStick[i]) == false)
			{
				GlowStick glowStickScript = SceneManager::GetSceneWithId(sceneId)->world->GetScript<GlowStick>(m_glowStick[i]);
				glowStickScript.DropGlowStick(t.local.GetPosition().x, t.local.GetPosition().y, t.local.GetPosition().z);
				break;
			}
		}
	}
}

void Movement::ScriptMovement::OnCollision(EntityId _otherId)
{
	Scene* s = SceneManager::GetSceneWithId(sceneId);
	if (s->world->HasComponent<ItemCollectableComponent>(_otherId) && s->world->IsActive(_otherId))
	{
		GameManager::CollectFood();
		s->world->SetInactive(_otherId);
	}
}

void Movement::ScriptMovement::Reload()
{
	TransformComponent& t = GetComponent<TransformComponent>();
	m_yaw = 0.0f;
	m_pitch = 0.0f;
	t.local.SetPosition(XMFLOAT3(0.0f, 1.0f, 0.0f));
	t.local.SetYPR(XMFLOAT3(m_yaw, m_pitch, 0.0f));

	for (int i = 0; i < 3; i++)
	{
		SceneManager::GetSceneWithId(sceneId)->world->SetInactive(m_glowStick[i]);
	}
}
