#include "GameWork_Gui.h"

#include <GLFW/glfw3.h>
#include "Base_imgui.h"
#include "Base_Input.h"

#include "Render_Facade.h"
#include "Render_Gui.h"

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

		myRenderGui = Render::Facade::GetInstance()->AddGui(myGuiContext);
	}

	Gui::~Gui()
	{
		Render::Facade::GetInstance()->RemoveGui(myRenderGui);

		ImGui::DestroyContext(myGuiContext);
	}

	void Gui::Update()
	{
		ImGui::SetCurrentContext(myGuiContext);
		ImGuiIO& io = ImGui::GetIO();

		int width, height;
		glfwGetWindowSize(myWindow, &width, &height);
		io.DisplaySize = ImVec2((float)width, (float)height);
		io.DeltaTime = 1.0f / 60.0f; // TODO

		double x, y;
		Input::InputManager::GetInstance()->PollMousePosition(x, y, myWindow);
		io.MousePos = ImVec2((float)x, (float)y);
		io.MouseDown[0] = Input::InputManager::GetInstance()->PollRawInput(Input::RawInput::MouseLeft, myWindow) == Input::RawInputState::Pressed;
		io.MouseDown[1] = Input::InputManager::GetInstance()->PollRawInput(Input::RawInput::MouseRight, myWindow) == Input::RawInputState::Pressed;

		ImGui::NewFrame();

		InternalUpdate();

		ImGui::Render();

		myRenderGui->Update();
	}

	void Gui::Draw()
	{
		Render::Facade::GetInstance()->GetRenderer(myWindow)->DrawGui(myRenderGui);
	}

}
