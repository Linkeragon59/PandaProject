#pragma once

#include "Render_Renderer.h"
#include "Render_Handle.h"

struct GLFWwindow;
struct ImGuiContext;

namespace Render
{
	void InitializeRendering();
	void FinalizeRendering();

	void StartFrame();
	void EndFrame();

	void RegisterWindow(GLFWwindow* aWindow, Renderer::Type aRendererType);
	void UnregisterWindow(GLFWwindow* aWindow);
	void ResizeWindow(GLFWwindow* aWindow);
	Renderer* GetRenderer(GLFWwindow* aWindow);

	Handle AddModel(const ModelData& someData);
	void RemoveModel(Handle aModelHandle);
	void UpdateModel(Handle aModelHandle, const ModelData& someData);

	Handle AddGui(ImGuiContext* aGuiContext);
	void RemoveGui(Handle aGuiHandle);
	void UpdateGui(Handle aGuiHandle);
}
