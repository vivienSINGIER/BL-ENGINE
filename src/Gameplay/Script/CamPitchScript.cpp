#include "CamPitchScript.h"
#include "../Gameplay/InventoryManager.h"

void CamPitchScript::Awake()
{
	m_pitch = 0.0f;
	m_cursorLocked = InputManager::IsMouseCursorLocked();
	InventoryManager::Init(entity);
}

void CamPitchScript::Update(float dt)
{
	m_cursorLocked = InputManager::IsMouseCursorLocked();
	TransformComponent& camT = GetComponent<TransformComponent>();
	float mouseSensitivity = 0.01f;
	XMFLOAT2 mouseDelta = InputManager::GetMouseDelta();
	m_pitch += mouseDelta.y * mouseSensitivity;

	m_pitch = Clamp(m_pitch, -59.0f * (XM_PI / 180.0f), 89.0f * (XM_PI / 180.0f));

	if(m_cursorLocked)
		camT.local.SetYPR(XMFLOAT3(0.0f, m_pitch, 0.0f));
}
