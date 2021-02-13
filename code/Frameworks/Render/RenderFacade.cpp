#include "RenderFacade.h"
#include "TriangleRenderer.h"

namespace Render
{
	TriangleRenderer* Facade::ourTriangleRenderer = nullptr;

	void Facade::RunTriangleRenderer()
	{
		ourTriangleRenderer = new Render::TriangleRenderer();
		ourTriangleRenderer->Run();
		delete ourTriangleRenderer;
	}
}
