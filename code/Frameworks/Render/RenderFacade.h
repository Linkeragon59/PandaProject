#pragma once

namespace Render
{
	class TriangleRenderer;

	class Facade
	{
	public:
		static void RunTriangleRenderer();

	private:
		static TriangleRenderer* ourTriangleRenderer;
	};
}
