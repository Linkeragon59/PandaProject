#pragma once
#include "GameCore_Module.h"

struct GLFWwindow;

namespace GameCore
{
	class CallbackGui;
}

namespace Editor
{
	class GraphEditorCanvas;

	class EditorModule : public GameCore::Module
	{
	DECLARE_GAMECORE_MODULE(EditorModule, "Editor")

	protected:
		void OnRegister() override;
		void OnUnregister() override;
		void OnUpdate(GameCore::Module::UpdateType aType) override;

	private:
		void Open();
		void Close();
		uint myOpenCloseCallbackId = UINT_MAX;

		void CallbackUpdate();

		GLFWwindow* myWindow = nullptr;
		GameCore::CallbackGui* myGui = nullptr;

		GraphEditorCanvas* myCanvas = nullptr;
	};
}
