#include "RenderFacade.h"
#include "TriangleRenderer.h"
#include "BasicRenderer.h"

namespace Render
{
	TriangleRenderer* Facade::ourTriangleRenderer = nullptr;
	BasicRenderer* Facade::ourBasicRenderer = nullptr;

	void Facade::RunTriangleRenderer()
	{
		ourTriangleRenderer = new TriangleRenderer();
		ourTriangleRenderer->Run();
		delete ourTriangleRenderer;
	}

	void Facade::RunBasicRenderer()
	{
		ourBasicRenderer = new BasicRenderer();
		ourBasicRenderer->Run();
		delete ourBasicRenderer;
	}

}
