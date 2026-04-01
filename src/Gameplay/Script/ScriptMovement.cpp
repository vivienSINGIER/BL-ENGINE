#include "ScriptMovement.h"

void Movement::ScriptMovement::Awake()
{
    m_yaw = 0.0f;
	m_pitch = 0.0f;
	InputManager::LockMouseCursor();
	InputManager::HideMouseCursor();
	bool cursorLocked = true;
}

void Movement::ScriptMovement::Update(float dt)
{
    TransformComponent& t = GetComponent<TransformComponent>();

	float mouseSensitivity = 0.01f;
	XMFLOAT2 mouseDelta = InputManager::GetMouseDelta();
	m_yaw += mouseDelta.x * mouseSensitivity;
	m_pitch += mouseDelta.y * mouseSensitivity;

	m_pitch = Clamp(m_pitch, -59.0f * (XM_PI / 180.0f), 89.0f * (XM_PI / 180.0f));

	if (m_cursorLocked)
	{
		t.local.SetYPR(XMFLOAT3(m_yaw, m_pitch, 0.0f));
	}

	float moveSpeed = 30.0f;

	XMFLOAT3 forward = t.local.GetForward();
	XMFLOAT3 right = t.local.GetRight();

	if (InputManager::IsKey(Z))
		t.local.Move(XMFLOAT3(forward.x * moveSpeed * dt, forward.y * moveSpeed * dt, forward.z * moveSpeed * dt));
	if (InputManager::IsKey(S))
		t.local.Move(XMFLOAT3(-forward.x * moveSpeed * dt, -forward.y * moveSpeed * dt, -forward.z * moveSpeed * dt));
	if (InputManager::IsKey(Q))
		t.local.Move(XMFLOAT3(-right.x * moveSpeed * dt, -right.y * moveSpeed * dt, -right.z * moveSpeed * dt));
	if (InputManager::IsKey(D))
		t.local.Move(XMFLOAT3(right.x * moveSpeed * dt, right.y * moveSpeed * dt, right.z * moveSpeed * dt));
	if(InputManager::IsKey(SPACE))
		t.local.Move(XMFLOAT3(0.0f, moveSpeed * dt, 0.0f));
	if (InputManager::IsKey(LCONTROL))
		t.local.Move(XMFLOAT3(0.0f, -moveSpeed * dt, 0.0f));
	if(InputManager::IsKeyDown(ESCAPE))
	{
		if(m_cursorLocked)
		{
			InputManager::UnlockMouseCursor();
			InputManager::ShowMouseCursor();
			m_cursorLocked = false;
		}
		else
		{
			InputManager::LockMouseCursor();
			InputManager::HideMouseCursor();
			m_cursorLocked = true;
		}
	}
}