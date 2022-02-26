#include "Render_RenderModule.h"

#include <GLFW/glfw3.h>

namespace Render
{
	DEFINE_GAMECORE_MODULE(RenderModule);

	void RenderModule::OnRegister()
	{
		myRenderCore = new RenderCore;
	}

	void RenderModule::OnUnregister()
	{
		SafeDelete(myRenderCore);
	}

	void RenderModule::OnInitialize()
	{
		myRenderCore->Initialize();
	}

	void RenderModule::OnFinalize()
	{
		myRenderCore->Finalize();
	}

	void RenderModule::OnUpdate(GameCore::Module::UpdateType aType)
	{
		if (aType == GameCore::Module::UpdateType::EarlyUpdate)
		{
			myRenderCore->StartFrame();
		}
		else if (aType == GameCore::Module::UpdateType::MainUpdate)
		{
			myRenderCore->Update();
		}
		else if (aType == GameCore::Module::UpdateType::LateUpdate)
		{
			myRenderCore->EndFrame();
		}
	}

	void RenderModule::RegisterWindow(GLFWwindow* aWindow, RendererType aType)
	{
		myRenderCore->RegisterWindow(aWindow, aType);
	}

	void RenderModule::UnregisterWindow(GLFWwindow* aWindow)
	{
		myRenderCore->UnregisterWindow(aWindow);
	}
}
