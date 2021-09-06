#include "GameWork_Editor.h"

#if DEBUG_BUILD

#include <GLFW/glfw3.h>

#include "GameWork.h"
#include "GameWork_WindowManager.h"
#include "GameWork_CallbackGui.h"

#include "Base_imgui.h"

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

		Render::RegisterWindow(myWindow, Render::Renderer::Type::Editor);

		myGui = new CallbackGui(myWindow, [&]() {
			int width, height;
			glfwGetWindowSize(myWindow, &width, &height);

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
				static char buf[32] = "hello";
				ImGui::InputText("Input", buf, 32);
				ImGui::Button("Test Button");
			}

			drawList->AddCircle(ImVec2(200.0f, 200.0f), 50.0f, ImGui::GetColorU32(IM_COL32(255, 255, 255, 255)), 0, 5.0f);

			ImGui::PopItemWidth();

			ImGui::End();
			ImGui::PopStyleVar();
		});
	}

	Editor::~Editor()
	{
		delete myGui;

		Render::UnregisterWindow(myWindow);

		GameWork::GetInstance()->GetWindowManager()->CloseWindow(myWindow);
	}

	void Editor::Update()
	{
		myGui->Update();
		myGui->Draw();
	}
}

#endif
