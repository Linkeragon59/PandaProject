#include "Editor_EditorModule.h"

#include <GLFW/glfw3.h>

#include "Base_imgui.h"
#include "Base_Input.h"
#include "Base_Window.h"

#include "Render_Facade.h"

#include "GameWork_CallbackGui.h"

#include "Editor_GraphEditorCanvas.h"

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
		myCanvas = new GraphEditorCanvas();
	}

	void EditorModule::Close()
	{
		SafeDelete(myCanvas);
		SafeDelete(myGui);

		Render::UnregisterWindow(myWindow);

		Window::WindowManager::GetInstance()->CloseWindow(myWindow);
		myWindow = nullptr;
	}

	void EditorModule::CallbackUpdate()
	{
		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->Pos);
		ImGui::SetNextWindowSize(viewport->Size);

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

		ImGui::Begin("Graph Editor", nullptr,
			ImGuiWindowFlags_AlwaysAutoResize |
			ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus |
			ImGuiWindowFlags_MenuBar
		);

		ImVec2 availableRegionPos = ImGui::GetCursorScreenPos();
		ImVec2 availableRegionSize = ImGui::GetContentRegionAvail();

		ImVec2 canvasPos = availableRegionPos;
		ImVec2 canvasSize = ImVec2(2.0f * availableRegionSize.x / 3.0f, availableRegionSize.y);

		ImVec2 propertiesPos = ImVec2(availableRegionPos.x + 2.0f * availableRegionSize.x / 3.0f, availableRegionPos.y);
		ImVec2 propertiesSize = ImVec2(availableRegionSize.x / 3.0f, availableRegionSize.y);

		ImGui::SetNextWindowPos(canvasPos);
		ImGui::BeginChild("canvas", canvasSize, true, ImGuiWindowFlags_NoScrollbar);
		{
			myCanvas->Draw(canvasPos, canvasSize);
		}
		ImGui::EndChild();

		ImGui::SetNextWindowPos(propertiesPos);
		ImGui::BeginChild("properties", propertiesSize);
		ImGui::EndChild();

		ImGui::PopStyleVar();

		ImGui::ShowDemoWindow();

		ImGui::End();
	}
}
