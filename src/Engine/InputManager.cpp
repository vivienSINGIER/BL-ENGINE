#ifndef INPUT_SYSTEM_CPP_INCLUDED
#define INPUT_SYSTEM_CPP_INCLUDED

#include "InputManager.h"

#include "EngineManager.h"
#include "Network/Client.h"
#include "Network/Packet.hpp"

UnorderedMap<UINT8, INT32> InputManager::s_keyboardMap{
    { BACKSPACE,       VK_BACK     },
    { TAB,             VK_TAB      },
    { RETURN,          VK_RETURN   },
    { PAUSE,           VK_PAUSE    },
    { CAPSLOCK,        VK_CAPITAL  },
    { ESCAPE,          VK_ESCAPE   },
    { SPACE,           VK_SPACE    },
    { PAGE_UP,         VK_PRIOR    },
    { PAGE_DOWN,       VK_NEXT     },
    { END,             VK_END      },
    { HOME,            VK_HOME     },
    { LEFT,            VK_LEFT     },
    { UP,              VK_UP       },
    { RIGHT,           VK_RIGHT    },
    { DOWN,            VK_DOWN     },
    { INSERT,          VK_INSERT   },
    { DELETE_,         VK_DELETE   },
    { LWINDOW,         VK_LWIN     },
    { RWINDOW,         VK_RWIN     },
    { NUMPAD0,         VK_NUMPAD0  },
    { NUMPAD1,         VK_NUMPAD1  },
    { NUMPAD2,         VK_NUMPAD2  },
    { NUMPAD3,         VK_NUMPAD3  },
    { NUMPAD4,         VK_NUMPAD4  },
    { NUMPAD5,         VK_NUMPAD5  },
    { NUMPAD6,         VK_NUMPAD6  },
    { NUMPAD7,         VK_NUMPAD7  },
    { NUMPAD8,         VK_NUMPAD8  },
    { NUMPAD9,         VK_NUMPAD9  },
    { NUMPAD_MULTIPLY, VK_MULTIPLY },
    { NUMPAD_ADD,      VK_ADD      },
    { NUMPAD_SUBTRACT, VK_SUBTRACT },
    { NUMPAD_DECIMAL,  VK_DECIMAL  },
    { NUMPAD_DIVIDE,   VK_DIVIDE   },
    { F1,              VK_F1       },
    { F2,              VK_F2       },
    { F3,              VK_F3       },
    { F4,              VK_F4       },
    { F5,              VK_F5       },
    { F6,              VK_F6       },
    { F7,              VK_F7       },
    { F8,              VK_F8       },
    { F9,              VK_F9       },
    { F10,             VK_F10      },
    { F11,             VK_F11      },
    { F12,             VK_F12      },
    { NUMLOCK,         VK_NUMLOCK  },
    { SCROLL_LOCK,     VK_SCROLL   },
    { LSHIFT,          VK_LSHIFT   },
    { RSHIFT,          VK_RSHIFT   },
    { LCONTROL,        VK_LCONTROL },
    { RCONTROL,        VK_RCONTROL },
    { LALT,            VK_LMENU    },
    { RALT,            VK_RMENU    },
    { A,              'A'          },
    { B,              'B'          },
    { C,              'C'          },
    { D,              'D'          },
    { E,              'E'          },
    { F,              'F'          },
    { G,              'G'          },
    { H,              'H'          },
    { I,              'I'          },
    { J,              'J'          },
    { K,              'K'          },
    { L,              'L'          },
    { M,              'M'          },
    { N,              'N'          },
    { O,              'O'          },
    { P,              'P'          },
    { Q,              'Q'          },
    { R,              'R'          },
    { S,              'S'          },
    { T,              'T'          },
    { U,              'U'          },
    { V,              'V'          },
    { W,              'W'          },
    { X,              'X'          },
    { Y,              'Y'          },
    { Z,              'Z'          },
    { _0,             '0'          },
    { _1,             '1'          },
    { _2,             '2'          },
    { _3,             '3'          },
    { _4,             '4'          },
    { _5,             '5'          },
    { _6,             '6'          },
    { _7,             '7'          },
    { _8,             '8'          },
    { _9,             '9'          },
    { ²,              VK_OEM_7     },
};

UnorderedMap<UINT8, INT32> InputManager::s_mouseMap{
    { LEFT_MOUSE,   VK_LBUTTON  },
    { RIGHT_MOUSE,  VK_RBUTTON  },
    { MIDDLE_MOUSE, VK_MBUTTON  }
};

void InputManager::RegisterClient(uint32 _clientId)
{
    if (s_keyboardStates.contains(_clientId) == false)
        s_keyboardStates[_clientId] = Array<InputState, AMOUNT_KEY>();
    if (s_mouseStates.contains(_clientId) == false)
        s_mouseStates[_clientId] = Array<InputState, AMOUNT_MOUSE>();
    if (s_mouseDatas.contains(_clientId) == false)
        s_mouseDatas[_clientId] = {};
}


void InputManager::Initialize(HWND pHWND)
{
	s_pHWND = pHWND;
}

void InputManager::HandleInput(uint32 _clientId)
{
    if (_clientId == 0)
        _clientId = EngineManager::GetClient()->GetId();

    RegisterClient(_clientId);

    Array<InputState, AMOUNT_KEY>& kb = s_keyboardStates[_clientId];
    Array<InputState, AMOUNT_MOUSE>& mouse = s_mouseStates[_clientId];
    MouseData& mData = s_mouseDatas[_clientId];
    
    if (GetForegroundWindow() != s_pHWND)
        return;
    
    for (const Pair<unsigned char, int> input : s_keyboardMap)
    {
        unsigned char inputKey = input.first;
        int indexKey = input.second;

        bool isKeyDown = (GetAsyncKeyState(indexKey) & 0x8000) != 0;
        InputState currentState = kb[inputKey];

        if (isKeyDown)
        {
            if (currentState == DOWN_STATE || currentState == PRESSED_STATE)
                kb[inputKey] = PRESSED_STATE;
            else
                kb[inputKey] = DOWN_STATE;
        }
        else
        {
            if (currentState == DOWN_STATE || currentState == PRESSED_STATE)
                kb[inputKey] = UP_STATE;
            else
                kb[inputKey] = NONE_STATE;
        }
    }

    for (const Pair<unsigned char, int> input : s_mouseMap)
    {
        unsigned char inputButton = input.first;
        int indexButton = input.second;

        bool isButtonDown = (GetAsyncKeyState(indexButton) & 0x8000) != 0;
        InputState currentState = mouse[inputButton];

        if (isButtonDown)
        {
            if (currentState == DOWN_STATE || currentState == PRESSED_STATE)
                mouse[inputButton] = PRESSED_STATE;
            else
                mouse[inputButton] = DOWN_STATE;
        }
        else
        {
            if (currentState == DOWN_STATE || currentState == PRESSED_STATE)
                mouse[inputButton] = UP_STATE;
            else
                mouse[inputButton] = NONE_STATE;
        }
    }

    POINT p;
    GetCursorPos(&p);
    ScreenToClient(s_pHWND, &p);

    XMINT2 newPos = { p.x, p.y };

    if (mData.cursorLocked)
    {
        RECT rect;
        GetClientRect(s_pHWND, &rect);

        int centerX = (rect.right - rect.left) / 2;
        int centerY = (rect.bottom - rect.top) / 2;

        mData.deltaX = newPos.x - centerX;
        mData.deltaY = newPos.y - centerY;

        POINT center = { centerX, centerY };
        ClientToScreen(s_pHWND, &center);
        SetCursorPos(center.x, center.y);

        mData.x = centerX;
        mData.y = centerY;
    }
    else
    {
        mData.deltaX = newPos.x - mData.x;
        mData.deltaY = newPos.y - mData.y;

        mData.x = newPos.x;
        mData.y = newPos.y;
    }

    mData.dirty = true;
}

bool InputManager::BuildKeyboardPacket(Packet& _p)
{
    uint32 clientId = EngineManager::GetClient()->GetId();
    RegisterClient(clientId);
    
    Array<InputState, AMOUNT_KEY>&  curr     = s_keyboardStates[clientId];
    Array<InputState, AMOUNT_KEY>&  last     = s_lastKeyboardStates[clientId];
    
    _p.header.type = PacketType::KeyUpdate;
    _p.input.inputCount = 0;
    
    for (int i = 0; i < AMOUNT_KEY; i++)
    {
        if (curr[i] == last[i]) continue;
        
        InputEntry entry;
        entry.keyCode = i;
        entry.state = curr[i];
        _p.input.inputs[_p.input.inputCount] = entry;
        _p.input.inputCount++;
        
        if (_p.input.inputCount >= MAX_INPUT_COUNT) break;
    }
    
    last = curr;
    
    return _p.input.inputCount > 0;
}

bool InputManager::BuildMousePacket(Packet& _p)
{
    uint32 clientId = EngineManager::GetClient()->GetId();
    RegisterClient(clientId);
    
    MouseData& data = s_mouseDatas[clientId];
    
    if (data.dirty == false) return false;
    
    _p.header.type = PacketType::MousePosUpdate;
    _p.mouse.x = data.x;
    _p.mouse.y = data.y;
    _p.mouse.dx = data.deltaX;
    _p.mouse.dy = data.deltaY;
    _p.mouse.cursorVisible = data.cursorVisible;
    _p.mouse.cursorLocked = data.cursorLocked;
    
    data.dirty = false; 
    
    return true;
}

void InputManager::UpdateRemoteStates()
{
    Client* client = EngineManager::GetClient();
    uint32 localId = UINT32_MAX;
    if (client != nullptr)
        localId = client->GetId();

    for (auto& [clientId, states] : s_keyboardStates)
    {
        if (clientId == localId) continue;

        for (int i = 0; i < AMOUNT_KEY; i++)
        {
            if (states[i] == DOWN_STATE) states[i] = PRESSED_STATE;
            if (states[i] == UP_STATE)   states[i] = NONE_STATE;
        }
    }
    
    for (auto& [clientId, states] : s_mouseStates)
    {
        if (clientId == localId) continue;

        for (int i = 0; i < AMOUNT_MOUSE; i++)
        {
            if (states[i] == DOWN_STATE) states[i] = PRESSED_STATE;
            if (states[i] == UP_STATE)   states[i] = NONE_STATE;
        }
    }
}

bool InputManager::BuildMouseButtonPacket(Packet& _p)
{
    uint32 clientId = EngineManager::GetClient()->GetId();
    RegisterClient(clientId);

    Array<InputState, AMOUNT_MOUSE>&  curr     = s_mouseStates[clientId];
    Array<InputState, AMOUNT_MOUSE>&  last     = s_lastMouseStates[clientId];
    
    _p.header.type = PacketType::MouseButtonUpdate;
    _p.input.inputCount = 0;
    
    for (int i = 0; i < AMOUNT_MOUSE; i++)
    {
        if (curr[i] == last[i]) continue;
        
        InputEntry entry;
        entry.keyCode = i;
        entry.state = curr[i];
        _p.input.inputs[_p.input.inputCount] = entry;
        _p.input.inputCount++;
        
        if (_p.input.inputCount >= MAX_INPUT_COUNT) break;
    }
    
    last = curr;
    
    return _p.input.inputCount > 0;
}

void InputManager::UpdateFromPacket(Packet& _p, uint32 _clientId)
{
    if (_clientId == 0)
        _clientId = EngineManager::GetClient()->GetId();
    RegisterClient(_clientId);

    if (_p.header.type == PacketType::MouseButtonUpdate)
    {
        for (int i = 0; i < _p.input.inputCount; i++)
        {
            SetMouseButtonState((InputMouse)_p.input.inputs[i].keyCode, _p.input.inputs[i].state, _clientId);
        }
    }
    
    if (_p.header.type == PacketType::KeyUpdate)
    {
        for (int i = 0; i < _p.input.inputCount; i++)
        {
            SetKeyState((InputKeyboard)_p.input.inputs[i].keyCode, _p.input.inputs[i].state, _clientId);
        }
    }
    
    if (_p.header.type == PacketType::MousePosUpdate)
    {
        MouseData& data = s_mouseDatas[_clientId];
        data.x = _p.mouse.x;
        data.y = _p.mouse.y;
        data.deltaX = _p.mouse.dx;
        data.deltaY = _p.mouse.dy;
        data.cursorVisible = _p.mouse.cursorVisible;
        data.cursorLocked = _p.mouse.cursorLocked;
    }
}

void InputManager::SetKeyState(InputKeyboard _key, InputState _state, uint32 _clientId)
{
    if (_clientId == 0)
        _clientId = EngineManager::GetClient()->GetId();
    RegisterClient(_clientId);
    
    s_keyboardStates[_clientId][_key] = _state;
}

bool InputManager::IsKey(InputKeyboard key, uint32 _clientId)
{
    if (_clientId == 0)
        _clientId = EngineManager::GetClient()->GetId();
    RegisterClient(_clientId);
    return s_keyboardStates[_clientId][key] == PRESSED_STATE;
}

bool InputManager::IsKeyUp(InputKeyboard key, uint32 _clientId)
{
    if (_clientId == 0)
        _clientId = EngineManager::GetClient()->GetId();
    RegisterClient(_clientId);
    return s_keyboardStates[_clientId][key] == UP_STATE;
}

bool InputManager::IsKeyDown(InputKeyboard key, uint32 _clientId)
{
    if (_clientId == 0)
        _clientId = EngineManager::GetClient()->GetId();
    RegisterClient(_clientId);
    return s_keyboardStates[_clientId][key] == DOWN_STATE;
}

bool InputManager::IsMouseButtonPressed(InputMouse key, uint32 _clientId)
{
    if (_clientId == 0)
        _clientId = EngineManager::GetClient()->GetId();
    RegisterClient(_clientId);
    
    return s_mouseStates[_clientId][key] == PRESSED_STATE;
}

bool InputManager::IsMouseButtonUp(InputMouse key, uint32 _clientId)
{
    if (_clientId == 0)
        _clientId = EngineManager::GetClient()->GetId();
    RegisterClient(_clientId);
    
    return s_mouseStates[_clientId][key] == UP_STATE;
}

bool InputManager::IsMouseButtonDown(InputMouse key, uint32 _clientId)
{
    if (_clientId == 0)
        _clientId = EngineManager::GetClient()->GetId();
    RegisterClient(_clientId);
    
    return s_mouseStates[_clientId][key] == DOWN_STATE;
}

void InputManager::SetMouseButtonState(InputMouse _key, InputState _state, uint32 _clientId)
{
    if (_clientId == 0)
        _clientId = EngineManager::GetClient()->GetId();
    RegisterClient(_clientId);
    
    s_mouseStates[_clientId][_key] = _state;
}

XMINT2 InputManager::GetMousePosition(uint32 _clientId)
{
    if (_clientId == 0)
        _clientId = EngineManager::GetClient()->GetId();
    RegisterClient(_clientId);
    
    return { s_mouseDatas[_clientId].x, s_mouseDatas[_clientId].y };
}

XMFLOAT2 InputManager::GetMouseDelta(uint32 _clientId)
{
    if (_clientId == 0)
        _clientId = EngineManager::GetClient()->GetId();
    RegisterClient(_clientId);
    
    return { s_mouseDatas[_clientId].deltaX, s_mouseDatas[_clientId].deltaY };
}

void InputManager::SetMousePosition( XMINT2 const& coordinates, uint32 _clientId )
{
    if (_clientId == 0)
        _clientId = EngineManager::GetClient()->GetId();
    RegisterClient(_clientId);
    
    POINT p{ coordinates.x, coordinates.y };
    ClientToScreen( s_pHWND, &p );
    SetCursorPos( p.x, p.y );
    
    s_mouseDatas[_clientId].x = coordinates.x;
    s_mouseDatas[_clientId].y = coordinates.y;
    s_mouseDatas[_clientId].dirty = true;
}

void InputManager::LockMouseCursor(uint32 _clientId)
{
    if (_clientId == 0)
        _clientId = EngineManager::GetClient()->GetId();
    RegisterClient(_clientId);
    
    if ( s_pHWND == nullptr ) return;
    s_mouseDatas[_clientId].cursorLocked = true;
    
    RECT clientRect;
    if ( GetClientRect( s_pHWND, &clientRect ) == false ) return;

    POINT topLeft = { clientRect.left, clientRect.top };
    POINT bottomRight = { clientRect.right, clientRect.bottom };

    ClientToScreen( s_pHWND, &topLeft );
    ClientToScreen( s_pHWND, &bottomRight );

    RECT const clipRect = { topLeft.x, topLeft.y, bottomRight.x, bottomRight.y };
    ClipCursor( &clipRect );
    s_mouseDatas[_clientId].dirty = true;
}

void InputManager::UnlockMouseCursor(uint32 _clientId)
{
    if (_clientId == 0)
        _clientId = EngineManager::GetClient()->GetId();
    RegisterClient(_clientId);
    
    s_mouseDatas[_clientId].cursorLocked = false;
    ClipCursor( nullptr );
    s_mouseDatas[_clientId].dirty = true;
}

bool InputManager::IsMouseCursorLocked(uint32 _clientId)
{
    if (_clientId == 0)
        _clientId = EngineManager::GetClient()->GetId();
    RegisterClient(_clientId);
    
    return s_mouseDatas[_clientId].cursorLocked;
}

void InputManager::ShowMouseCursor(uint32 _clientId)
{
    if (_clientId == 0)
        _clientId = EngineManager::GetClient()->GetId();
    RegisterClient(_clientId);
    
    if (  s_mouseDatas[_clientId].cursorVisible ) return;
    s_mouseDatas[_clientId].cursorVisible = true;

    while ( s_mouseDatas[_clientId].cursorVisibilityCount < 0 )
        s_mouseDatas[_clientId].cursorVisibilityCount = ShowCursor( TRUE );
    s_mouseDatas[_clientId].dirty = true;
}

void InputManager::HideMouseCursor(uint32 _clientId)
{
    if (_clientId == 0)
        _clientId = EngineManager::GetClient()->GetId();
    RegisterClient(_clientId);
    
    if ( !s_mouseDatas[_clientId].cursorVisible ) return;
    s_mouseDatas[_clientId].cursorVisible = false;

    while ( s_mouseDatas[_clientId].cursorVisibilityCount >= 0 )
        s_mouseDatas[_clientId].cursorVisibilityCount = ShowCursor( FALSE );
    s_mouseDatas[_clientId].dirty = true;
}

bool InputManager::IsMouseCursorVisible(uint32 _clientId)
{
    if (_clientId == 0)
        _clientId = EngineManager::GetClient()->GetId();
    RegisterClient(_clientId);
    
    return s_mouseDatas[_clientId].cursorVisible;
}

#endif
