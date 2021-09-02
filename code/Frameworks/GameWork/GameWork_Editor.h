#pragma once

#if DEBUG_BUILD

struct GLFWwindow;
struct ImGuiContext;

namespace GameWork
{
	class Editor
	{
	public:
		enum class Tab
		{
			ConsoleLog,
			GraphEditor,
		};

		Editor();
		~Editor();

		void Update();

		void SetTab(Tab aTab) { myTab = aTab; }
		Tab GetTab() const { return myTab; }

	private:
		Tab myTab = Tab::ConsoleLog;

		GLFWwindow* myWindow = nullptr;
		ImGuiContext* myGuiContext = nullptr;
	};
}

#endif
