#pragma once

#if DEBUG_BUILD

struct GLFWwindow;

namespace GameWork
{
	class CallbackGui;

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
		CallbackGui* myGui = nullptr;
	};
}

#endif
