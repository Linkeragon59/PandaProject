#include "GameWork_Editor.h"

#if DEBUG_BUILD

#include <GLFW/glfw3.h>

#include "GameWork.h"
#include "GameWork_WindowManager.h"

#include "Base_imgui.h"
#include "Base_Input.h"

#include "Render_Facade.h"

namespace GameWork
{
	namespace
	{
		const uint locWindowWidth = 1280;
		const uint locWindowHeight = 720;
	}

	Editor::Editor()
	{
		Assert(GameWork::GetInstance() && GameWork::GetInstance()->GetWindowManager());
		myWindow = GameWork::GetInstance()->GetWindowManager()->OpenWindow(locWindowWidth, locWindowHeight, "Editor");

		Render::Facade::GetInstance()->RegisterWindow(myWindow, Render::Renderer::Type::Editor);

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

		Render::Facade::GetInstance()->GetRenderer(myWindow)->DrawGui(myGuiContext);
	}

	Editor::~Editor()
	{
		ImGui::DestroyContext(myGuiContext);

		Render::Facade::GetInstance()->UnregisterWindow(myWindow);

		GameWork::GetInstance()->GetWindowManager()->CloseWindow(myWindow);
	}

	void Editor::Update()
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

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
		ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
		ImGui::SetNextWindowSize(ImVec2((float)width, (float)height), ImGuiCond_Always);
		ImGui::Begin("Vulkan Example", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

		ImGui::PushItemWidth(110.0f);

		ImDrawList* drawList = ImGui::GetWindowDrawList();

		if (ImGui::CollapsingHeader("Subpasses", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::Text("Test Gui");
		}
		if (ImGui::CollapsingHeader("Subpasses", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::Button("Test Button");
		}

		drawList->AddCircle(ImVec2(200.0f, 200.0f), 50.0f, ImGui::GetColorU32(IM_COL32(255, 255, 255, 255)), 0, 5.0f);

		ImGui::PopItemWidth();

		ImGui::End();
		ImGui::PopStyleVar();
		ImGui::Render();

		Render::Facade::GetInstance()->GetRenderer(myWindow)->DrawGui(myGuiContext);
	}
}

#endif
