#pragma once

namespace Render
{
	class TriangleRenderer;
	class BasicRendererTuto;
	class BasicRenderer;

	class Facade
	{
	public:
		static void RunTriangleRenderer();
		static void RunBasicRendererTuto();
		static void RunBasicRenderer();

		static void RunVulkanRenderer();

	private:
		static TriangleRenderer* ourTriangleRenderer;
		static BasicRendererTuto* ourBasicRendererTuto;
		static BasicRenderer* ourBasicRenderer;
	};
}
