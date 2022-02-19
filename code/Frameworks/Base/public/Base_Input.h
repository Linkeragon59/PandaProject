#pragma once

#include <functional>

#include "Base_SlotVector.h"

struct GLFWwindow;

namespace Window
{
	class WindowManager;
}

namespace Input
{
	enum MouseButton
	{
		MouseLeft,
		MouseRight,
		MouseMiddle,

		MouseCount,
	};

	enum Key
	{
		Key0,
		Key1,
		Key2,
		Key3,
		Key4,
		Key5,
		Key6,
		Key7,
		Key8,
		Key9,

		KeyA,
		KeyB,
		KeyC,
		KeyD,
		KeyE,
		KeyF,
		KeyG,
		KeyH,
		KeyI,
		KeyJ,
		KeyK,
		KeyL,
		KeyM,
		KeyN,
		KeyO,
		KeyP,
		KeyQ,
		KeyR,
		KeyS,
		KeyT,
		KeyU,
		KeyV,
		KeyW,
		KeyX,
		KeyY,
		KeyZ,

		KeySpace,
		KeyApostrophe,
		KeyGraveAccent,
		KeyComma,
		KeySemicolon,
		KeyPeriod,
		KeyHyphen,
		KeyEqual,
		KeySlash,
		KeyBackSlash,
		KeyLeftBracket,
		KeyRightBracket,

		KeyEscape,
		KeyEnter,
		KeyTab,
		KeyBackspace,
		KeyInsert,
		KeyDelete,
		KeyRight,
		KeyLeft,
		KeyDown,
		KeyUp,
		KeyPageUp,
		KeyPageDown,
		KeyHome,
		KeyEnd,
		KeyCapsLock,
		KeyScrollLock,
		KeyNumLock,
		KeyPrintScreen,
		KeyPause,
		KeyF1,
		KeyF2,
		KeyF3,
		KeyF4,
		KeyF5,
		KeyF6,
		KeyF7,
		KeyF8,
		KeyF9,
		KeyF10,
		KeyF11,
		KeyF12,
		KeyLeftShift,
		KeyLeftCtrl,
		KeyLeftAlt,
		KeyLeftSuper,
		KeyRightShift,
		KeyRightCtrl,
		KeyRightAlt,
		KeyRightSuper,
		KeyMenu,

		KeyNumPad0,
		KeyNumPad1,
		KeyNumPad2,
		KeyNumPad3,
		KeyNumPad4,
		KeyNumPad5,
		KeyNumPad6,
		KeyNumPad7,
		KeyNumPad8,
		KeyNumPad9,
		KeyNumPadDecimal,
		KeyNumPadDivide,
		KeyNumPadMultiply,
		KeyNumPadSubtract,
		KeyNumPadAdd,
		KeyNumPadEnter,
		KeyNumPadEqual,

		KeyCount,
	};

	enum class Status
	{
		Released,
		Pressed,
		Repeated,
		Unknown,
	};

	enum Modifier
	{
		ModNone		= 0x00,
		ModShift	= 0x01,
		ModControl	= 0x02,
		ModAlt		= 0x04,
		ModSuper	= 0x08,
		ModCapsLock	= 0x10,
		ModNumLock	= 0x20,
	};

	typedef std::function<void(Status, Modifier)> MouseCallback;
	struct MouseCallbackEntry
	{
		void Clear() { myWindow = nullptr; myCallback = nullptr; }
		bool IsSet() const { return myCallback != nullptr; }
		MouseButton myButton;
		GLFWwindow* myWindow = nullptr;
		MouseCallback myCallback = nullptr;
	};
	
	typedef std::function<void(Status, Modifier)> KeyCallback;
	struct KeyCallbackEntry
	{
		void Clear() { myWindow = nullptr; myCallback = nullptr; }
		bool IsSet() const { return myCallback != nullptr; }
		Key myKey;
		GLFWwindow* myWindow = nullptr;
		KeyCallback myCallback = nullptr;
	};

	typedef std::function<void(double, double)> ScrollCallback;
	struct ScrollCallbackEntry
	{
		void Clear() { myWindow = nullptr; myCallback = nullptr; }
		bool IsSet() const { return myCallback != nullptr; }
		GLFWwindow* myWindow = nullptr;
		ScrollCallback myCallback = nullptr;
	};

	typedef std::function<void(uint)> CharacterCallback;
	struct CharacterCallbackEntry
	{
		void Clear() { myWindow = nullptr; myCallback = nullptr; }
		bool IsSet() const { return myCallback != nullptr; }
		GLFWwindow* myWindow = nullptr;
		CharacterCallback myCallback = nullptr;
	};

	class InputManager
	{
	public:
		static void Create();
		static void Destroy();
		static InputManager* GetInstance() { return ourInstance; }

		Status PollMouseInput(MouseButton aButton, GLFWwindow* aWindow = nullptr) const;
		Status PollKeyInput(Key aKey, GLFWwindow* aWindow = nullptr) const;
		void PollMousePosition(double& anOutX, double& anOutY, GLFWwindow* aWindow = nullptr) const;

		uint AddMouseCallback(MouseButton aButton, MouseCallback aCallback, GLFWwindow* aWindow = nullptr);
		void RemoveMouseCallback(uint aCallbakId);

		uint AddKeyCallback(Key aKey, KeyCallback aCallback, GLFWwindow* aWindow = nullptr);
		void RemoveKeyCallback(uint aCallbakId);

		uint AddScrollCallback(ScrollCallback aCallback, GLFWwindow* aWindow = nullptr);
		void RemoveScrollCallback(uint aCallbakId);

		uint AddCharacterCallback(CharacterCallback aCallback, GLFWwindow* aWindow = nullptr);
		void RemoveCharacterCallback(uint aCallbakId);

	protected:
		friend class Window::WindowManager;
		static void OnMouseCallback(GLFWwindow* aWindow, int aButton, int anAction, int someMods);
		static void OnKeyCallback(GLFWwindow* aWindow, int aKey, int aScanCode, int anAction, int someMods);
		static void OnScrollCallback(GLFWwindow* aWindow, double anX, double anY);
		static void OnCharacterCallback(GLFWwindow* aWindow, uint aUnicodeCodePoint);

	private:
		static InputManager* ourInstance;

		SlotVector<MouseCallbackEntry> myMouseCallbacks;
		SlotVector<KeyCallbackEntry> myKeyCallbacks;
		SlotVector<ScrollCallbackEntry> myScrollCallbacks;
		SlotVector<CharacterCallbackEntry> myCharacterCallbacks;
	};
} // namespace Input
