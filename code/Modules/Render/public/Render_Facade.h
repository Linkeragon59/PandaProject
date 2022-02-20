#pragma once

#include "Render_Renderer.h"
#include "Render_Handle.h"

struct GLFWwindow;
struct ImGuiContext;

namespace Render
{
	Handle AddModel(const ModelData& someData);
	void RemoveModel(Handle aModelHandle);
	void UpdateModel(Handle aModelHandle, const ModelData& someData);

	Handle AddGui(ImGuiContext* aGuiContext);
	void RemoveGui(Handle aGuiHandle);
	void UpdateGui(Handle aGuiHandle);
}
