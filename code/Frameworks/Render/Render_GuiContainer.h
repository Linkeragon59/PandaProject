#pragma once

#include "Render_Handle.h"

struct ImGuiContext;

namespace Render
{
	class Gui;

	class GuiContainer
	{
	public:
		~GuiContainer();

		Handle AddGui(ImGuiContext* aGuiContext);
		void RemoveGui(Handle aGuiHandle);
		void UpdateGui(Handle aGuiHandle);

		Gui* GetGui(Handle aGuiHandle) const;

	private:
		std::vector<Gui*> myGuis; // TODO : store available slots in a list
	};
}