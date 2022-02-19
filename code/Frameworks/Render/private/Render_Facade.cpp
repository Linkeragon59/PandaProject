#include "Render_Facade.h"

#include "Render_ModelContainer.h"
#include "Render_GuiContainer.h"

namespace Render
{
	void InitializeRendering()
	{
		RenderCore::Create();
	}

	void FinalizeRendering()
	{
		RenderCore::Destroy();
	}

	void StartFrame()
	{
		RenderCore::GetInstance()->StartFrame();
	}

	void EndFrame()
	{
		RenderCore::GetInstance()->EndFrame();
	}

	void RegisterWindow(GLFWwindow* aWindow, Renderer::Type aRendererType)
	{
		RenderCore::GetInstance()->RegisterWindow(aWindow, aRendererType);
	}

	void UnregisterWindow(GLFWwindow* aWindow)
	{
		RenderCore::GetInstance()->UnregisterWindow(aWindow);
	}

	Renderer* GetRenderer(GLFWwindow* aWindow)
	{
		return RenderCore::GetInstance()->GetRenderer(aWindow);
	}

	Handle AddModel(const ModelData& someData)
	{
		return RenderCore::GetInstance()->GetModelContainer()->AddModel(someData);
	}

	void RemoveModel(Handle aModelHandle)
	{
		RenderCore::GetInstance()->GetModelContainer()->RemoveModel(aModelHandle);
	}

	void UpdateModel(Handle aModelHandle, const ModelData& someData)
	{
		RenderCore::GetInstance()->GetModelContainer()->UpdateModel(aModelHandle, someData);
	}

	Handle AddGui(ImGuiContext* aGuiContext)
	{
		return RenderCore::GetInstance()->GetGuiContainer()->AddGui(aGuiContext);
	}

	void RemoveGui(Handle aGuiHandle)
	{
		RenderCore::GetInstance()->GetGuiContainer()->RemoveGui(aGuiHandle);
	}

	void UpdateGui(Handle aGuiHandle)
	{
		RenderCore::GetInstance()->GetGuiContainer()->UpdateGui(aGuiHandle);
	}
}
