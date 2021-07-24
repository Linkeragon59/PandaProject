#pragma once

struct ImGuiContext;

namespace GameWork
{
	class Gui
	{
	public:
		Gui();
		~Gui();

		void Update();

	private:
		ImGuiContext* myGuiContext = nullptr;
	};
}
