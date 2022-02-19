#include "GameWork_Gui.h"

#include <GLFW/glfw3.h>
#include "Base_imgui.h"
#include "Base_Input.h"
#include "Base_Window.h"
#include "Base_Time.h"

#include "Render_Facade.h"

namespace GameWork
{
	Gui::Gui(GLFWwindow* aWindow)
		: myWindow(aWindow)
	{
		myGuiContext = ImGui::CreateContext();

		glfwGetWindowSize(myWindow, &myWindowWidth, &myWindowHeight);
		myWindowResizeCallbackId = Window::WindowManager::GetInstance()->AddWindowSizeCallback([this](int aWidth, int aHeight) {
			myWindowWidth = aWidth;
			myWindowHeight = aHeight;
		}, myWindow);

		myScrollCallbackId = Input::InputManager::GetInstance()->AddScrollCallback([this](double aXScroll, double aYScroll) {
			myXScroll = aXScroll;
			myYScroll = aYScroll;
		}, myWindow);

		myCharacterCallbackId = Input::InputManager::GetInstance()->AddCharacterCallback([this](uint aUnicodeCodePoint) {
			myTextInput.push(aUnicodeCodePoint);
		}, myWindow);

		myGui = Render::AddGui(myGuiContext);

		InitStyle();
		InitIO();
	}

	Gui::~Gui()
	{
		Window::WindowManager::GetInstance()->RemoveWindowSizeCallback(myWindowResizeCallbackId);
		Input::InputManager::GetInstance()->RemoveScrollCallback(myScrollCallbackId);
		Input::InputManager::GetInstance()->RemoveCharacterCallback(myCharacterCallbackId);

		Render::RemoveGui(myGui);

		ImGui::DestroyContext(myGuiContext);
	}

	void Gui::Update()
	{
		ImGui::SetCurrentContext(myGuiContext);
		ImGuiIO& io = ImGui::GetIO();

		io.DisplaySize = ImVec2((float)myWindowWidth, (float)myWindowHeight);
		io.DeltaTime = Time::TimeManager::GetInstance()->GetDeltaTime();

		double x, y;
		Input::InputManager::GetInstance()->PollMousePosition(x, y, myWindow);
		io.MousePos = ImVec2((float)x, (float)y);

		io.MouseDown[ImGuiMouseButton_Left] = Input::InputManager::GetInstance()->PollMouseInput(Input::MouseLeft, myWindow) == Input::Status::Pressed;
		io.MouseDown[ImGuiMouseButton_Right] = Input::InputManager::GetInstance()->PollMouseInput(Input::MouseRight, myWindow) == Input::Status::Pressed;
		io.MouseDown[ImGuiMouseButton_Middle] = Input::InputManager::GetInstance()->PollMouseInput(Input::MouseMiddle, myWindow) == Input::Status::Pressed;

		io.MouseWheelH = (float)myXScroll;
		io.MouseWheel = (float)myYScroll;
		myXScroll = myYScroll = 0.0;

		io.KeyCtrl = Input::InputManager::GetInstance()->PollKeyInput(Input::KeyLeftCtrl, myWindow) == Input::Status::Pressed
			|| Input::InputManager::GetInstance()->PollKeyInput(Input::KeyRightCtrl, myWindow) == Input::Status::Pressed;
		io.KeyShift = Input::InputManager::GetInstance()->PollKeyInput(Input::KeyLeftShift, myWindow) == Input::Status::Pressed
			|| Input::InputManager::GetInstance()->PollKeyInput(Input::KeyRightShift, myWindow) == Input::Status::Pressed;
		io.KeyAlt = Input::InputManager::GetInstance()->PollKeyInput(Input::KeyLeftAlt, myWindow) == Input::Status::Pressed
			|| Input::InputManager::GetInstance()->PollKeyInput(Input::KeyRightAlt, myWindow) == Input::Status::Pressed;
		io.KeySuper = Input::InputManager::GetInstance()->PollKeyInput(Input::KeyLeftSuper, myWindow) == Input::Status::Pressed
			|| Input::InputManager::GetInstance()->PollKeyInput(Input::KeyRightSuper, myWindow) == Input::Status::Pressed;

		for (uint key = 0; key < ImGuiKey_COUNT; ++key)
		{
			io.KeysDown[io.KeyMap[key]] = Input::InputManager::GetInstance()->PollKeyInput((Input::Key)io.KeyMap[key], myWindow) == Input::Status::Pressed;
		}

		while (!myTextInput.empty())
		{
			io.AddInputCharacter(myTextInput.front());
			myTextInput.pop();
		}

		ImGui::NewFrame();

		InternalUpdate();

		ImGui::Render();

		Render::UpdateGui(myGui);
	}

	void Gui::Draw()
	{
		Render::GetRenderer(myWindow)->DrawGui(myGui);
	}

	void Gui::InitStyle()
	{
		ImGuiStyle& style = ImGui::GetStyle();

		style.Colors[ImGuiCol_TitleBg] = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
		style.Colors[ImGuiCol_TitleBgActive] = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
		style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.0f, 0.0f, 0.0f, 0.1f);
		style.Colors[ImGuiCol_MenuBarBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
		style.Colors[ImGuiCol_Header] = ImVec4(0.8f, 0.0f, 0.0f, 0.4f);
		style.Colors[ImGuiCol_HeaderActive] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
		style.Colors[ImGuiCol_HeaderHovered] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
		style.Colors[ImGuiCol_FrameBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.8f);
		style.Colors[ImGuiCol_CheckMark] = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);
		style.Colors[ImGuiCol_SliderGrab] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
		style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);
		style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(1.0f, 1.0f, 1.0f, 0.1f);
		style.Colors[ImGuiCol_FrameBgActive] = ImVec4(1.0f, 1.0f, 1.0f, 0.2f);
		style.Colors[ImGuiCol_Button] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
		style.Colors[ImGuiCol_ButtonHovered] = ImVec4(1.0f, 0.0f, 0.0f, 0.6f);
		style.Colors[ImGuiCol_ButtonActive] = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);
	}

	void Gui::InitIO()
	{
		ImGuiIO& io = ImGui::GetIO();

		// For now, disable the .ini files, as it is not useful so far
		io.IniFilename = nullptr;

		io.DisplaySize = ImVec2((float)myWindowWidth, (float)myWindowHeight);

		io.KeyMap[ImGuiKey_Tab] = Input::KeyTab;
		io.KeyMap[ImGuiKey_LeftArrow] = Input::KeyLeft;
		io.KeyMap[ImGuiKey_RightArrow] = Input::KeyRight;
		io.KeyMap[ImGuiKey_UpArrow] = Input::KeyUp;
		io.KeyMap[ImGuiKey_DownArrow] = Input::KeyDown;
		io.KeyMap[ImGuiKey_PageUp] = Input::KeyPageUp;
		io.KeyMap[ImGuiKey_PageDown] = Input::KeyPageDown;
		io.KeyMap[ImGuiKey_Home] = Input::KeyHome;
		io.KeyMap[ImGuiKey_End] = Input::KeyEnd;
		io.KeyMap[ImGuiKey_Insert] = Input::KeyInsert;
		io.KeyMap[ImGuiKey_Delete] = Input::KeyDelete;
		io.KeyMap[ImGuiKey_Backspace] = Input::KeyBackspace;
		io.KeyMap[ImGuiKey_Space] = Input::KeySpace;
		io.KeyMap[ImGuiKey_Enter] = Input::KeyEnter;
		io.KeyMap[ImGuiKey_Escape] = Input::KeyEscape;
		io.KeyMap[ImGuiKey_KeyPadEnter] = Input::KeyNumPadEnter;
		io.KeyMap[ImGuiKey_A] = Input::KeyA;
		io.KeyMap[ImGuiKey_C] = Input::KeyC;
		io.KeyMap[ImGuiKey_V] = Input::KeyV;
		io.KeyMap[ImGuiKey_X] = Input::KeyX;
		io.KeyMap[ImGuiKey_Y] = Input::KeyY;
		io.KeyMap[ImGuiKey_Z] = Input::KeyZ;
	}
}
