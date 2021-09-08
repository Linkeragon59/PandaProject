#include "Base_Input.h"

#include <GLFW/glfw3.h>

namespace Input
{
	namespace
	{
		int locMouseButtonToGlfwMapping[MouseCount];
		int locKeyToGlfwMapping[KeyCount];

		void locInitInputToGlfwMapping()
		{
			locMouseButtonToGlfwMapping[MouseLeft] = GLFW_MOUSE_BUTTON_LEFT;
			locMouseButtonToGlfwMapping[MouseRight] = GLFW_MOUSE_BUTTON_RIGHT;
			locMouseButtonToGlfwMapping[MouseMiddle] = GLFW_MOUSE_BUTTON_MIDDLE;

			locKeyToGlfwMapping[Key0] = GLFW_KEY_0;
			locKeyToGlfwMapping[Key1] = GLFW_KEY_1;
			locKeyToGlfwMapping[Key2] = GLFW_KEY_2;
			locKeyToGlfwMapping[Key3] = GLFW_KEY_3;
			locKeyToGlfwMapping[Key4] = GLFW_KEY_4;
			locKeyToGlfwMapping[Key5] = GLFW_KEY_5;
			locKeyToGlfwMapping[Key6] = GLFW_KEY_6;
			locKeyToGlfwMapping[Key7] = GLFW_KEY_7;
			locKeyToGlfwMapping[Key8] = GLFW_KEY_8;
			locKeyToGlfwMapping[Key9] = GLFW_KEY_9;

			locKeyToGlfwMapping[KeyA] = GLFW_KEY_A;
			locKeyToGlfwMapping[KeyB] = GLFW_KEY_B;
			locKeyToGlfwMapping[KeyC] = GLFW_KEY_C;
			locKeyToGlfwMapping[KeyD] = GLFW_KEY_D;
			locKeyToGlfwMapping[KeyE] = GLFW_KEY_E;
			locKeyToGlfwMapping[KeyF] = GLFW_KEY_F;
			locKeyToGlfwMapping[KeyG] = GLFW_KEY_G;
			locKeyToGlfwMapping[KeyH] = GLFW_KEY_H;
			locKeyToGlfwMapping[KeyI] = GLFW_KEY_I;
			locKeyToGlfwMapping[KeyJ] = GLFW_KEY_J;
			locKeyToGlfwMapping[KeyK] = GLFW_KEY_K;
			locKeyToGlfwMapping[KeyL] = GLFW_KEY_L;
			locKeyToGlfwMapping[KeyM] = GLFW_KEY_M;
			locKeyToGlfwMapping[KeyN] = GLFW_KEY_N;
			locKeyToGlfwMapping[KeyO] = GLFW_KEY_O;
			locKeyToGlfwMapping[KeyP] = GLFW_KEY_P;
			locKeyToGlfwMapping[KeyQ] = GLFW_KEY_Q;
			locKeyToGlfwMapping[KeyR] = GLFW_KEY_R;
			locKeyToGlfwMapping[KeyS] = GLFW_KEY_S;
			locKeyToGlfwMapping[KeyT] = GLFW_KEY_T;
			locKeyToGlfwMapping[KeyU] = GLFW_KEY_U;
			locKeyToGlfwMapping[KeyV] = GLFW_KEY_V;
			locKeyToGlfwMapping[KeyW] = GLFW_KEY_W;
			locKeyToGlfwMapping[KeyX] = GLFW_KEY_X;
			locKeyToGlfwMapping[KeyY] = GLFW_KEY_Y;
			locKeyToGlfwMapping[KeyZ] = GLFW_KEY_Z;

			locKeyToGlfwMapping[KeySpace] = GLFW_KEY_SPACE;
			locKeyToGlfwMapping[KeyApostrophe] = GLFW_KEY_APOSTROPHE;
			locKeyToGlfwMapping[KeyGraveAccent] = GLFW_KEY_GRAVE_ACCENT;
			locKeyToGlfwMapping[KeyComma] = GLFW_KEY_COMMA;
			locKeyToGlfwMapping[KeySemicolon] = GLFW_KEY_SEMICOLON;
			locKeyToGlfwMapping[KeyPeriod] = GLFW_KEY_PERIOD;
			locKeyToGlfwMapping[KeyHyphen] = GLFW_KEY_MINUS;
			locKeyToGlfwMapping[KeyEqual] = GLFW_KEY_EQUAL;
			locKeyToGlfwMapping[KeySlash] = GLFW_KEY_SLASH;
			locKeyToGlfwMapping[KeyBackSlash] = GLFW_KEY_BACKSLASH;
			locKeyToGlfwMapping[KeyLeftBracket] = GLFW_KEY_LEFT_BRACKET;
			locKeyToGlfwMapping[KeyRightBracket] = GLFW_KEY_RIGHT_BRACKET;

			locKeyToGlfwMapping[KeyEscape] = GLFW_KEY_ESCAPE;
			locKeyToGlfwMapping[KeyEnter] = GLFW_KEY_ENTER;
			locKeyToGlfwMapping[KeyTab] = GLFW_KEY_TAB;
			locKeyToGlfwMapping[KeyBackspace] = GLFW_KEY_BACKSPACE;
			locKeyToGlfwMapping[KeyInsert] = GLFW_KEY_INSERT;
			locKeyToGlfwMapping[KeyDelete] = GLFW_KEY_DELETE;
			locKeyToGlfwMapping[KeyRight] = GLFW_KEY_RIGHT;
			locKeyToGlfwMapping[KeyLeft] = GLFW_KEY_LEFT;
			locKeyToGlfwMapping[KeyDown] = GLFW_KEY_DOWN;
			locKeyToGlfwMapping[KeyUp] = GLFW_KEY_UP;
			locKeyToGlfwMapping[KeyPageUp] = GLFW_KEY_PAGE_UP;
			locKeyToGlfwMapping[KeyPageDown] = GLFW_KEY_PAGE_DOWN;
			locKeyToGlfwMapping[KeyHome] = GLFW_KEY_HOME;
			locKeyToGlfwMapping[KeyEnd] = GLFW_KEY_END;
			locKeyToGlfwMapping[KeyCapsLock] = GLFW_KEY_CAPS_LOCK;
			locKeyToGlfwMapping[KeyScrollLock] = GLFW_KEY_SCROLL_LOCK;
			locKeyToGlfwMapping[KeyNumLock] = GLFW_KEY_NUM_LOCK;
			locKeyToGlfwMapping[KeyPrintScreen] = GLFW_KEY_PRINT_SCREEN;
			locKeyToGlfwMapping[KeyPause] = GLFW_KEY_PAUSE;
			locKeyToGlfwMapping[KeyF1] = GLFW_KEY_F1;
			locKeyToGlfwMapping[KeyF2] = GLFW_KEY_F2;
			locKeyToGlfwMapping[KeyF3] = GLFW_KEY_F3;
			locKeyToGlfwMapping[KeyF4] = GLFW_KEY_F4;
			locKeyToGlfwMapping[KeyF5] = GLFW_KEY_F5;
			locKeyToGlfwMapping[KeyF6] = GLFW_KEY_F6;
			locKeyToGlfwMapping[KeyF7] = GLFW_KEY_F7;
			locKeyToGlfwMapping[KeyF8] = GLFW_KEY_F8;
			locKeyToGlfwMapping[KeyF9] = GLFW_KEY_F9;
			locKeyToGlfwMapping[KeyF10] = GLFW_KEY_F10;
			locKeyToGlfwMapping[KeyF11] = GLFW_KEY_F11;
			locKeyToGlfwMapping[KeyF12] = GLFW_KEY_F12;
			locKeyToGlfwMapping[KeyLeftShift] = GLFW_KEY_LEFT_SHIFT;
			locKeyToGlfwMapping[KeyLeftCtrl] = GLFW_KEY_LEFT_CONTROL;
			locKeyToGlfwMapping[KeyLeftAlt] = GLFW_KEY_LEFT_ALT;
			locKeyToGlfwMapping[KeyLeftSuper] = GLFW_KEY_LEFT_SUPER;
			locKeyToGlfwMapping[KeyRightShift] = GLFW_KEY_RIGHT_SHIFT;
			locKeyToGlfwMapping[KeyRightCtrl] = GLFW_KEY_RIGHT_CONTROL;
			locKeyToGlfwMapping[KeyRightAlt] = GLFW_KEY_RIGHT_ALT;
			locKeyToGlfwMapping[KeyRightSuper] = GLFW_KEY_RIGHT_SUPER;
			locKeyToGlfwMapping[KeyMenu] = GLFW_KEY_MENU;

			locKeyToGlfwMapping[KeyNumPad0] = GLFW_KEY_KP_0;
			locKeyToGlfwMapping[KeyNumPad1] = GLFW_KEY_KP_1;
			locKeyToGlfwMapping[KeyNumPad2] = GLFW_KEY_KP_2;
			locKeyToGlfwMapping[KeyNumPad3] = GLFW_KEY_KP_3;
			locKeyToGlfwMapping[KeyNumPad4] = GLFW_KEY_KP_4;
			locKeyToGlfwMapping[KeyNumPad5] = GLFW_KEY_KP_5;
			locKeyToGlfwMapping[KeyNumPad6] = GLFW_KEY_KP_6;
			locKeyToGlfwMapping[KeyNumPad7] = GLFW_KEY_KP_7;
			locKeyToGlfwMapping[KeyNumPad8] = GLFW_KEY_KP_8;
			locKeyToGlfwMapping[KeyNumPad9] = GLFW_KEY_KP_9;
			locKeyToGlfwMapping[KeyNumPadDecimal] = GLFW_KEY_KP_DECIMAL;
			locKeyToGlfwMapping[KeyNumPadDivide] = GLFW_KEY_KP_DIVIDE;
			locKeyToGlfwMapping[KeyNumPadMultiply] = GLFW_KEY_KP_MULTIPLY;
			locKeyToGlfwMapping[KeyNumPadSubtract] = GLFW_KEY_KP_SUBTRACT;
			locKeyToGlfwMapping[KeyNumPadAdd] = GLFW_KEY_KP_ADD;
			locKeyToGlfwMapping[KeyNumPadEnter] = GLFW_KEY_KP_ENTER;
			locKeyToGlfwMapping[KeyNumPadEqual] = GLFW_KEY_KP_EQUAL;
		}

		Status locGlfwToInputStatus(int aGlfwAction)
		{
			if (aGlfwAction == GLFW_RELEASE)
				return Status::Released;
			else if (aGlfwAction == GLFW_PRESS)
				return Status::Pressed;
			else if (aGlfwAction == GLFW_REPEAT)
				return Status::Repeated;
			else
				return Status::Unknown;
		}

		Modifier locGlfwToModifier(int someGlfwMods)
		{
			uint mods = ModNone;
			if (someGlfwMods & GLFW_MOD_SHIFT)
				mods &= ModShift;
			if (someGlfwMods & GLFW_MOD_CONTROL)
				mods &= ModControl;
			if (someGlfwMods & GLFW_MOD_ALT)
				mods &= ModShift;
			if (someGlfwMods & GLFW_MOD_SUPER)
				mods &= ModSuper;
			if (someGlfwMods & GLFW_MOD_CAPS_LOCK)
				mods &= ModCapsLock;
			if (someGlfwMods & GLFW_MOD_NUM_LOCK)
				mods &= ModNumLock;
			return Modifier(mods);
		}
	}

	InputManager* InputManager::ourInstance = nullptr;

	void InputManager::Create()
	{
		ourInstance = new InputManager;
		locInitInputToGlfwMapping();
	}
	
	void InputManager::Destroy()
	{
		SafeDelete(ourInstance);
	}

	Status InputManager::PollMouseInput(MouseButton aButton, GLFWwindow* aWindow /*= nullptr*/) const
	{
		GLFWwindow* window = aWindow ? aWindow : myMainWindow;
		if (!window)
			return Status::Unknown;

		return locGlfwToInputStatus(glfwGetMouseButton(window, locMouseButtonToGlfwMapping[aButton]));
	}

	Status InputManager::PollKeyInput(Key aKey, GLFWwindow* aWindow /*= nullptr*/) const
	{
		GLFWwindow* window = aWindow ? aWindow : myMainWindow;
		if (!window)
			return Status::Unknown;

		return locGlfwToInputStatus(glfwGetKey(window, locKeyToGlfwMapping[aKey]));
	}

	void InputManager::PollMousePosition(double& anOutX, double& anOutY, GLFWwindow* aWindow /*= nullptr*/) const
	{
		GLFWwindow* window = aWindow ? aWindow : myMainWindow;
		if (!window)
		{
			anOutX = anOutY = -1;
			return;
		}

		glfwGetCursorPos(window, &anOutX, &anOutY);
	}

	uint InputManager::AddMouseCallback(MouseButton aButton, MouseCallback aCallback, GLFWwindow* aWindow /*= nullptr*/)
	{
		GLFWwindow* window = aWindow ? aWindow : myMainWindow;
		if (!window)
			return UINT_MAX;

		MouseCallbackEntry entry;
		entry.myButton = aButton;
		entry.myWindow = window;
		entry.myCallback = aCallback;
		return myMouseCallbacks.Add(entry);
	}

	void InputManager::RemoveMouseCallback(uint aCallbakId)
	{
		myMouseCallbacks.Remove(aCallbakId);
	}

	uint InputManager::AddKeyCallback(Key aKey, KeyCallback aCallback, GLFWwindow* aWindow /*= nullptr*/)
	{
		GLFWwindow* window = aWindow ? aWindow : myMainWindow;
		if (!window)
			return UINT_MAX;

		KeyCallbackEntry entry;
		entry.myKey = aKey;
		entry.myWindow = window;
		entry.myCallback = aCallback;
		return myKeyCallbacks.Add(entry);
	}

	void InputManager::RemoveKeyCallback(uint aCallbakId)
	{
		myKeyCallbacks.Remove(aCallbakId);
	}

	uint InputManager::AddScrollCallback(ScrollCallback aCallback, GLFWwindow* aWindow /*= nullptr*/)
	{
		GLFWwindow* window = aWindow ? aWindow : myMainWindow;
		if (!window)
			return UINT_MAX;

		ScrollCallbackEntry entry;
		entry.myWindow = window;
		entry.myCallback = aCallback;
		return myScrollCallbacks.Add(entry);
	}

	void InputManager::RemoveScrollCallback(uint aCallbakId)
	{
		myScrollCallbacks.Remove(aCallbakId);
	}

	uint InputManager::AddCharacterCallback(CharacterCallback aCallback, GLFWwindow* aWindow /*= nullptr*/)
	{
		GLFWwindow* window = aWindow ? aWindow : myMainWindow;
		if (!window)
			return UINT_MAX;

		CharacterCallbackEntry entry;
		entry.myWindow = window;
		entry.myCallback = aCallback;
		return myCharacterCallbacks.Add(entry);
	}

	void InputManager::RemoveCharacterCallback(uint aCallbakId)
	{
		myCharacterCallbacks.Remove(aCallbakId);
	}

	void InputManager::OnMouseCallback(GLFWwindow* aWindow, int aButton, int anAction, int someMods)
	{
		for(const MouseCallbackEntry& entry : ourInstance->myMouseCallbacks.myEntries)
		{
			if (entry.IsSet() && aWindow == entry.myWindow && aButton == locMouseButtonToGlfwMapping[entry.myButton])
			{
				entry.myCallback(locGlfwToInputStatus(anAction), locGlfwToModifier(someMods));
			}
		}
	}

	void InputManager::OnKeyCallback(GLFWwindow* aWindow, int aKey, int aScanCode, int anAction, int someMods)
	{
		(void)aScanCode;

		for (const KeyCallbackEntry& entry : ourInstance->myKeyCallbacks.myEntries)
		{
			if (entry.IsSet() && aWindow == entry.myWindow && aKey == locKeyToGlfwMapping[entry.myKey])
			{
				entry.myCallback(locGlfwToInputStatus(anAction), locGlfwToModifier(someMods));
			}
		}
	}

	void InputManager::OnScrollCallback(GLFWwindow* aWindow, double anX, double anY)
	{
		for (const ScrollCallbackEntry& entry : ourInstance->myScrollCallbacks.myEntries)
		{
			if (entry.IsSet() && aWindow == entry.myWindow)
			{
				entry.myCallback(anX, anY);
			}
		}
	}

	void InputManager::OnCharacterCallback(GLFWwindow* aWindow, uint aUnicodeCodePoint)
	{
		for (const CharacterCallbackEntry& entry : ourInstance->myCharacterCallbacks.myEntries)
		{
			if (entry.IsSet() && aWindow == entry.myWindow)
			{
				entry.myCallback(aUnicodeCodePoint);
			}
		}
	}
}
