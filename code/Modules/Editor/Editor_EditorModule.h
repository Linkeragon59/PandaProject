#pragma once
#include "GameWork_Module.h"

struct GLFWwindow;

namespace GameWork
{
	class CallbackGui;
}

namespace Editor
{
	class EditorModule : public GameWork::Module
	{
	DECLARE_GAMEWORK_MODULE(EditorModule, "Editor")
	protected:
		void OnRegister() override;
		void OnUnregister() override;
		void OnUpdate() override;

	private:
		void Open();
		void Close();
		uint myOpenCloseCallbackId = UINT_MAX;

		void CallbackUpdate();

		GLFWwindow* myWindow = nullptr;
		GameWork::CallbackGui* myGui = nullptr;
	};
}
