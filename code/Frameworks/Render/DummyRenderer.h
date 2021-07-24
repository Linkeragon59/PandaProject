#pragma once

struct GLFWwindow;

namespace Render
{
	class DummyRenderer
	{
	public:
		DummyRenderer();
		~DummyRenderer();
		void OpenWindow();
		void PrintExtensionsCount();
		void TestMatrixCalculation();
		void Run();
		void CloseWindow();

	private:
		GLFWwindow* myWindow;
	};
}
