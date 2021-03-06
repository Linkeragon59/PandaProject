#include "RenderFacade.h"
#include "BasicRenderer.h"

namespace Render
{
	BasicRenderer* Facade::ourBasicRenderer = nullptr;

	void Facade::InitBasicRenderer(GLFWwindow* aWindow)
	{
		ourBasicRenderer = new BasicRenderer(aWindow);
	}

	void Facade::UpdateBasicRenderer()
	{
		ourBasicRenderer->Update();
	}

	void Facade::FinalizeBasicRenderer()
	{
		delete ourBasicRenderer;
	}
}
