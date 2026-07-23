#include "ButtonSystem.h"

#include "EngineManager.h"
#include "InputManager.h"
#include "RessourceManager.h"
#include "Generic/RenderItems/Sprite.h"

void ButtonSystem::OnUpdate(float _dt, EntityId _e, UiButtonComponent& _button)
{
    if (_button.m_isInit == false)
        Init(_button);

    _button.isHovered = IsHovered(_button);
    _button.isPressed = _button.isHovered && InputManager::IsMouseButtonPressed(InputMouse::LEFT_MOUSE);

    if (_button.isDisabled)
        TransitionState(_button, UiButtonComponent::ButtonStateType::DISABLED);
    else if (_button.isPressed   && _button.currState != UiButtonComponent::ButtonStateType::PRESSED)
        TransitionState(_button, UiButtonComponent::ButtonStateType::PRESSED);
    else if (_button.isHovered && _button.isPressed == false && _button.currState != UiButtonComponent::ButtonStateType::HOVERED)
        TransitionState(_button, UiButtonComponent::ButtonStateType::HOVERED);
    else if (_button.currState != UiButtonComponent::ButtonStateType::IDLE && _button.isHovered == false && _button.isPressed   == false)
        TransitionState(_button, UiButtonComponent::ButtonStateType::IDLE);
    
    Draw(_button);
}

void ButtonSystem::Init(UiButtonComponent& _button)
{
    Sprite* sp = RessourceManager::GetSprite(_button.spriteId);
    if (sp == nullptr) return;

    _button.m_halfWidth = sp->GetBounds().distanceX;
    _button.m_halfHeight = sp->GetBounds().distanceY;
        
    _button.m_isInit = true;
}

bool ButtonSystem::IsHovered(UiButtonComponent& _button)
{
    if (_button.m_isInit == false) return false;
    
    XMINT2 mousePos = InputManager::GetCenteredMousePosition();
    Vect2f32 pos = _button.transform.GetPosition();
    Vect2f32 scale = _button.transform.GetScale();
    
    int dX = abs(mousePos.x - (int)pos.x);
    int dY = abs(mousePos.y - (int)pos.y);

    if (dX > (int)((float)_button.m_halfWidth * scale.x))
        return false;
    if (dY > (int)((float)_button.m_halfWidth * scale.y))
        return false;
    
    return true;
}

void ButtonSystem::TransitionState(UiButtonComponent& _b, UiButtonComponent::ButtonStateType _newState)
{
    UiButtonComponent::ButtonState& currState = _b.states[_b.currState];
    UiButtonComponent::ButtonState& newState = _b.states[_newState];

    _b.transform.Move({-currState.statePos.x, -currState.statePos.y});
    _b.transform.Scale({1.0f / currState.stateScale.x, 1.0f / currState.stateScale.y});
    _b.transform.Rotate(-currState.stateRotation);

    _b.transform.Move({newState.statePos.x, newState.statePos.y});
    _b.transform.Scale({newState.stateScale.x, newState.stateScale.y});
    _b.transform.Rotate(newState.stateRotation);

    _b.currState = _newState;
}

void ButtonSystem::Draw(UiButtonComponent& _button)
{
    Sprite* sp = RessourceManager::GetSprite(_button.spriteId);

    UiButtonComponent::ButtonState& currState = _button.states[_button.currState]; 
    UiMaterial* uiMat = RessourceManager::GetUiMaterial(currState.materialId);
    if (uiMat == nullptr)
        uiMat = RessourceManager::GetUiMaterial(_button.states[UiButtonComponent::ButtonStateType::IDLE].materialId);

    Mat4f32 t = _button.transform.GetMatrix();
    
    if (sp == nullptr) return;
    if (uiMat == nullptr) return;
    
    Device* pDevice = EngineManager::GetDevice();
    if (pDevice == nullptr) return;
    
    pDevice->SetUiMaterial(uiMat);
    pDevice->DrawUi(sp, t);
}
