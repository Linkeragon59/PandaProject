#pragma once

namespace Render
{
	class TriangleRenderer;
	class BasicRenderer;

	class Facade
	{
	public:
		static void RunTriangleRenderer();
		static void RunBasicRenderer();

	private:
		static TriangleRenderer* ourTriangleRenderer;
		static BasicRenderer* ourBasicRenderer;
	};
}
