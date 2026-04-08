#ifndef INPUT_SYSTEM_H_INCLUDED
#define INPUT_SYSTEM_H_INCLUDED

#include "define.h"
#include "Network/Packet.hpp"

enum InputKeyboard 
{
    F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
    
    A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,

    _0, _1, _2, _3, _4, _5, _6, _7, _8, _9,
    NUMPAD0, NUMPAD1, NUMPAD2, NUMPAD3, NUMPAD4, NUMPAD5, NUMPAD6, NUMPAD7, NUMPAD8, NUMPAD9,
    NUMPAD_DIVIDE, NUMPAD_MULTIPLY, NUMPAD_SUBTRACT, NUMPAD_ADD, NUMPAD_RETURN, NUMPAD_DECIMAL,

    NUMLOCK, CAPSLOCK, SCROLL_LOCK,
    PAUSE,

    ²,

    LCTRL, RCTRL,
    LSHIFT, RSHIFT,
    LALT, RALT,
    LWINDOW, RWINDOW,

    UP, LEFT, DOWN, RIGHT,

    ESCAPE, TAB, SPACE, BACKSPACE, RETURN,
    INSERT, DELETE_,
    HOME, END,
    PAGE_UP, PAGE_DOWN,

    AMOUNT_KEY,

    ESC = ESCAPE,
    SPACEBAR = SPACE,

    ENTER = RETURN,
    NUMPAD_ENTER = NUMPAD_RETURN,

    LCONTROL = LCTRL,
    RCONTROL = RCTRL,
    ALTGR = RALT,

    UP_ARROW = UP,
    LEFT_ARROW = LEFT,
    DOWN_ARROW = DOWN,
    RIGHT_ARROW = RIGHT,
};

enum InputMouse 
{
    LEFT_MOUSE,
    RIGHT_MOUSE,
    MIDDLE_MOUSE,
    AMOUNT_MOUSE
};

struct MouseData
{
    bool cursorLocked = false;
    bool cursorVisible = true;
    int cursorVisibilityCount = 0;
    
    int32 x, y;
    float deltaX, deltaY;
    
    bool dirty = false;
};

class InputManager
{
public:
    InputManager() = default;
    ~InputManager() = default;

    static void Initialize(HWND hwnd);

    static void HandleInput(uint32 _clientId = 0);
    static void UpdateRemoteStates();
    
    static bool BuildKeyboardPacket(Packet& _p);
    static bool BuildMousePacket(Packet& _p);
    static bool BuildMouseButtonPacket(Packet& _p);
    
    static void UpdateFromPacket(Packet& _p, uint32 _clientId = 0);
    
    static void SetKeyState(InputKeyboard _key, InputState _state, uint32 _clientId = 0);
    
    static bool IsKeyDown(InputKeyboard _key, uint32 _clientId = 0);
    static bool IsKey(InputKeyboard _key, uint32 _clientId = 0);
    static bool IsKeyUp(InputKeyboard _key, uint32 _clientId = 0);
    
    static bool IsMouseButtonPressed(InputMouse _key, uint32 _clientId = 0);
    static bool IsMouseButtonUp(InputMouse _key, uint32 _clientId = 0);
    static bool IsMouseButtonDown(InputMouse _key, uint32 _clientId = 0);
    
    static void SetMouseButtonState(InputMouse _key, InputState _state, uint32 _clientId = 0);
    
    static XMINT2 GetMousePosition(uint32 _clientId = 0);
    static XMFLOAT2 GetMouseDelta(uint32 _clientId = 0);
    static void SetMousePosition(XMINT2 const& coordinates, uint32 _clientId = 0);
    
    static void LockMouseCursor(uint32 _clientId = 0);
    static void UnlockMouseCursor(uint32 _clientId = 0);
    static bool IsMouseCursorLocked(uint32 _clientId = 0);
    
    static void ShowMouseCursor(uint32 _clientId = 0);
    static void HideMouseCursor(uint32 _clientId = 0);
    static bool IsMouseCursorVisible(uint32 _clientId = 0);

    static bool IsCursorLocked();

private:
    static UnorderedMap<UINT8, INT32> s_keyboardMap;
    static UnorderedMap<UINT8, INT32> s_mouseMap;
    
    inline static UnorderedMap<uint32, Array<InputState, AMOUNT_KEY>> s_keyboardStates;
    inline static UnorderedMap<uint32, Array<InputState, AMOUNT_KEY>>  s_lastKeyboardStates;
    
    inline static UnorderedMap<uint32, Array<InputState, AMOUNT_MOUSE>> s_mouseStates;
    inline static UnorderedMap<uint32, Array<InputState, AMOUNT_MOUSE>> s_lastMouseStates;
    inline static UnorderedMap<uint32, MouseData> s_mouseDatas;
    
    inline static HWND s_pHWND = nullptr;
    
    static void RegisterClient(uint32 _clientId);

    static bool m_locked;
};


#endif
