#include "GameWork_Gui.h"

#include <GLFW/glfw3.h>
#include "Base_imgui.h"
#include "Base_Input.h"

#include "Render_Facade.h"

namespace GameWork
{
	Gui::Gui(GLFWwindow* aWindow)
		: myWindow(aWindow)
	{
		myGuiContext = ImGui::CreateContext();

		// Color scheme
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

		ImGuiIO& io = ImGui::GetIO();
		{
			// TODO properly
			int width, height;
			//glfwGetMonitorPhysicalSize(glfwGetPrimaryMonitor(), &width, &height);
			glfwGetWindowSize(myWindow, &width, &height);
			io.DisplaySize = ImVec2((float)width, (float)height);
			
			// TODO
			io.DeltaTime = 1.0f / 60.0f;
			
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

			myScrollCallbackId = Input::InputManager::GetInstance()->AddScrollCallback(std::bind(&Gui::ScrollCallback, this, std::placeholders::_1, std::placeholders::_2), myWindow);
			myCharacterCallbackId = Input::InputManager::GetInstance()->AddCharacterCallback(std::bind(&Gui::CharacterCallback, this, std::placeholders::_1), myWindow);
		}

		myGui = Render::AddGui(myGuiContext);
	}

	Gui::~Gui()
	{
		Input::InputManager::GetInstance()->RemoveScrollCallback(myScrollCallbackId);
		Input::InputManager::GetInstance()->RemoveCharacterCallback(myCharacterCallbackId);

		Render::RemoveGui(myGui);

		ImGui::DestroyContext(myGuiContext);
	}

	void Gui::Update()
	{
		ImGui::SetCurrentContext(myGuiContext);
		ImGuiIO& io = ImGui::GetIO();

		double x, y;
		Input::InputManager::GetInstance()->PollMousePosition(x, y, myWindow);
		io.MousePos = ImVec2((float)x, (float)y);

		io.MouseDown[0] = Input::InputManager::GetInstance()->PollMouseInput(Input::MouseLeft, myWindow) == Input::Status::Pressed;
		io.MouseDown[1] = Input::InputManager::GetInstance()->PollMouseInput(Input::MouseRight, myWindow) == Input::Status::Pressed;

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

		io.KeysDown[Input::KeyTab] = Input::InputManager::GetInstance()->PollKeyInput(Input::KeyTab, myWindow) == Input::Status::Pressed;
		io.KeysDown[Input::KeyLeft] = Input::InputManager::GetInstance()->PollKeyInput(Input::KeyLeft, myWindow) == Input::Status::Pressed;;
		io.KeysDown[Input::KeyRight] = Input::InputManager::GetInstance()->PollKeyInput(Input::KeyRight, myWindow) == Input::Status::Pressed;
		io.KeysDown[Input::KeyUp] = Input::InputManager::GetInstance()->PollKeyInput(Input::KeyUp, myWindow) == Input::Status::Pressed;
		io.KeysDown[Input::KeyDown] = Input::InputManager::GetInstance()->PollKeyInput(Input::KeyDown, myWindow) == Input::Status::Pressed;
		io.KeysDown[Input::KeyPageUp] = Input::InputManager::GetInstance()->PollKeyInput(Input::KeyPageUp, myWindow) == Input::Status::Pressed;
		io.KeysDown[Input::KeyPageDown] = Input::InputManager::GetInstance()->PollKeyInput(Input::KeyPageDown, myWindow) == Input::Status::Pressed;
		io.KeysDown[Input::KeyHome] = Input::InputManager::GetInstance()->PollKeyInput(Input::KeyHome, myWindow) == Input::Status::Pressed;
		io.KeysDown[Input::KeyEnd] = Input::InputManager::GetInstance()->PollKeyInput(Input::KeyEnd, myWindow) == Input::Status::Pressed;
		io.KeysDown[Input::KeyInsert] = Input::InputManager::GetInstance()->PollKeyInput(Input::KeyInsert, myWindow) == Input::Status::Pressed;
		io.KeysDown[Input::KeyDelete] = Input::InputManager::GetInstance()->PollKeyInput(Input::KeyDelete, myWindow) == Input::Status::Pressed;
		io.KeysDown[Input::KeyBackspace] = Input::InputManager::GetInstance()->PollKeyInput(Input::KeyBackspace, myWindow) == Input::Status::Pressed;
		io.KeysDown[Input::KeySpace] = Input::InputManager::GetInstance()->PollKeyInput(Input::KeySpace, myWindow) == Input::Status::Pressed;
		io.KeysDown[Input::KeyEnter] = Input::InputManager::GetInstance()->PollKeyInput(Input::KeyEnter, myWindow) == Input::Status::Pressed;
		io.KeysDown[Input::KeyEscape] = Input::InputManager::GetInstance()->PollKeyInput(Input::KeyEscape, myWindow) == Input::Status::Pressed;
		io.KeysDown[Input::KeyNumPadEnter] = Input::InputManager::GetInstance()->PollKeyInput(Input::KeyNumPadEnter, myWindow) == Input::Status::Pressed;
		io.KeysDown[Input::KeyA] = Input::InputManager::GetInstance()->PollKeyInput(Input::KeyA, myWindow) == Input::Status::Pressed;
		io.KeysDown[Input::KeyC] = Input::InputManager::GetInstance()->PollKeyInput(Input::KeyC, myWindow) == Input::Status::Pressed;
		io.KeysDown[Input::KeyV] = Input::InputManager::GetInstance()->PollKeyInput(Input::KeyV, myWindow) == Input::Status::Pressed;
		io.KeysDown[Input::KeyX] = Input::InputManager::GetInstance()->PollKeyInput(Input::KeyX, myWindow) == Input::Status::Pressed;
		io.KeysDown[Input::KeyY] = Input::InputManager::GetInstance()->PollKeyInput(Input::KeyY, myWindow) == Input::Status::Pressed;
		io.KeysDown[Input::KeyZ] = Input::InputManager::GetInstance()->PollKeyInput(Input::KeyZ, myWindow) == Input::Status::Pressed;

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

	void Gui::ScrollCallback(double aX, double aY)
	{
		myXScroll = aX;
		myYScroll = aY;
	}

	void Gui::CharacterCallback(uint aUnicodeCodePoint)
	{
		myTextInput.push(aUnicodeCodePoint);
	}
}