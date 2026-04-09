#include "ScriptMovement.h"
#include "../Gameplay/GlowStick.h"
#include "../Gameplay/Scene/MainScene.h"
#include "../Gameplay/InventoryManager.h"
#include "../Gameplay/PlayerHealth.hpp"
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

	m_healthText = SceneManager::GetSceneWithId(sceneId)->world->CreateEntity();
	TextComponent& healthText = SceneManager::GetSceneWithId(sceneId)->world->AddComponent<TextComponent>(m_healthText);
	healthText.textId = RessourceManager::GetTextId("HealthLabel");
	healthText.transform.SetPosition(XMFLOAT2(-900.0f, 350.0f));
	SceneManager::GetSceneWithId(sceneId)->world->SetInactive(m_healthText);
}

void Movement::ScriptMovement::Update(float dt)
{
	InventoryManager::Update(dt);
	m_cursorLocked = InputManager::IsMouseCursorLocked();
	TransformComponent& t = GetComponent<TransformComponent>();
	MotionComponent& motion = GetComponent<MotionComponent>();

	PlayerHealthComponent& hp = GetComponent<PlayerHealthComponent>();
	std::string hpText = std::to_string((int)hp.GetHealth());
	TextComponent& healthText = SceneManager::GetSceneWithId(sceneId)->world->GetComponent<TextComponent>(m_healthText);
	healthText.SetText(hpText);

	float mouseSensitivity = 0.005f;
	XMFLOAT2 mouseDelta = InputManager::GetMouseDelta();
	m_yaw += mouseDelta.x * mouseSensitivity;
	if (m_cursorLocked)
		t.local.SetYPR(XMFLOAT3(m_yaw, m_pitch, 0.0f));

	float moveSpeed = 30.0f;
	float maxHorizontalSpeed = 8.0f;

	XMFLOAT3 forward = t.local.GetForward();
	XMFLOAT3 right = t.local.GetRight();

	float curHSpeed = sqrtf(motion.linearVelocity.x * motion.linearVelocity.x + motion.linearVelocity.z * motion.linearVelocity.z);

	if (curHSpeed < maxHorizontalSpeed) 
	{
		if (InputManager::IsKey(Z))
			motion.AddLinearImpulse(XMFLOAT3(forward.x * moveSpeed * dt, 0.0f, forward.z * moveSpeed * dt));
		if (InputManager::IsKey(S))
			motion.AddLinearImpulse(XMFLOAT3(-forward.x * moveSpeed * dt, 0.0f, -forward.z * moveSpeed * dt));
		if (InputManager::IsKey(Q))
			motion.AddLinearImpulse(XMFLOAT3(-right.x * moveSpeed * dt, 0.0f, -right.z * moveSpeed * dt));
		if (InputManager::IsKey(D))
			motion.AddLinearImpulse(XMFLOAT3(right.x * moveSpeed * dt, 0.0f, right.z * moveSpeed * dt));
	}

	if (InputManager::IsKeyDown(SPACE) && m_isGrounded && !m_jumpConsumed)
	{
		motion.linearVelocity.y = 5.0f;
		m_isGrounded = false;
		m_jumpConsumed = true;
	}
	if (m_isGrounded)
		m_jumpConsumed = false;

	m_isGrounded = false; 

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

	if (InputManager::IsKeyDown(_1))
	{
		m_itemInHand = InventoryManager::TakeItem(0);
	}
	if (InputManager::IsKeyDown(_2))
	{
		m_itemInHand = InventoryManager::TakeItem(1);
	}
	if (InputManager::IsKeyDown(_3))
	{
		m_itemInHand = InventoryManager::TakeItem(2);
	}
	if (InputManager::IsKeyDown(E))
	{
		if(m_itemInHand != 0)
		{
			ItemCollectableComponent& item = SceneManager::GetSceneWithId(sceneId)->world->GetComponent<ItemCollectableComponent>(m_itemInHand);
			if (item.type == ItemType::Medic)
			{
				PlayerHealthComponent& hp = GetComponent<PlayerHealthComponent>();
				hp.Heal(30.0f);
				SceneManager::GetSceneWithId(sceneId)->world->SetInactive(m_itemInHand);
				InventoryManager::RemoveItem(m_itemInHand);
				m_itemInHand = 0;
			}
		}
	}

	if(m_itemInHand != 0 && InputManager::IsMouseButtonPressed(InputMouse::LEFT_MOUSE))
	{
		m_throwStrength += dt * 50.0f;
	}
	if(m_itemInHand != 0 && InputManager::IsMouseButtonUp(InputMouse::LEFT_MOUSE))
	{
		InventoryManager::ThrowSelectedItem(m_throwStrength);
		m_throwStrength = 10.0f;
		MotionComponent& itemMotion = SceneManager::GetSceneWithId(sceneId)->world->GetComponent<MotionComponent>(m_itemInHand);
		m_itemInHand = 0;
	}
}

void Movement::ScriptMovement::OnCollision(EntityId _otherId)
{
	Scene* s = SceneManager::GetSceneWithId(sceneId);
	if (m_itemInHand == _otherId) return;

	TransformComponent& myT = GetComponent<TransformComponent>();
	TransformComponent& otherT = s->world->GetComponent<TransformComponent>(_otherId);

	if (otherT.world.GetPosition().y < myT.world.GetPosition().y)
		m_isGrounded = true;

	if (s->world->HasComponent<ItemCollectableComponent>(_otherId) && s->world->IsActive(_otherId) && InventoryManager::FullInventory() == false)
	{
		InventoryManager::AddItem(_otherId);
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
	m_itemInHand = 0;
	
	for (int i = 0; i < 3; i++)
	{
		SceneManager::GetSceneWithId(sceneId)->world->SetInactive(m_glowStick[i]);
	}
}
