#include "Editor_EditorModule.h"

#include <GLFW/glfw3.h>

#include "Base_imgui.h"
#include "Base_Input.h"
#include "Base_Window.h"

#include "Render_Facade.h"

#include "GameWork_CallbackGui.h"

namespace Editor
{
	EditorModule* EditorModule::ourInstance = nullptr;

	void EditorModule::OnRegister()
	{
		myOpenCloseCallbackId = Input::InputManager::GetInstance()->AddKeyCallback(Input::KeyF1, [this](Input::Status aStatus, Input::Modifier someModifiers) {
			(void)someModifiers;
			if (aStatus == Input::Status::Pressed)
			{
				if (myWindow == nullptr)
					Open();
				else
					Close();
			}
		});
	}

	void EditorModule::OnUnregister()
	{
		if (myWindow != nullptr)
			Close();

		Input::InputManager::GetInstance()->RemoveKeyCallback(myOpenCloseCallbackId);
	}

	void EditorModule::OnUpdate()
	{
		if (!myWindow)
			return;

		if (glfwWindowShouldClose(myWindow))
		{
			Close();
			return;
		}

		myGui->Update();
		myGui->Draw();
	}

	void EditorModule::Open()
	{
		myWindow = Window::WindowManager::GetInstance()->OpenWindow("Editor");

		Render::RegisterWindow(myWindow, Render::Renderer::Type::Editor);

		myGui = new GameWork::CallbackGui(myWindow, std::bind(&EditorModule::CallbackUpdate, this));
	}

	void EditorModule::Close()
	{
		SafeDelete(myGui);

		Render::UnregisterWindow(myWindow);

		Window::WindowManager::GetInstance()->CloseWindow(myWindow);
		myWindow = nullptr;
	}

	void EditorModule::CallbackUpdate()
	{
		int width, height;
		glfwGetWindowSize(myWindow, &width, &height);

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
		ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
		ImGui::SetNextWindowSize(ImVec2((float)width, (float)height), ImGuiCond_Always);
		ImGui::ShowDemoWindow();
		/*ImGui::Begin("Vulkan Example", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

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

		ImGui::End();*/
		ImGui::PopStyleVar();
	}
}
