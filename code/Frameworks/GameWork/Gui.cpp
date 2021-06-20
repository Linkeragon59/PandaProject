#include "Gui.h"

#include "imgui.h"

namespace GameWork
{
	Gui::Gui()
	{
		myGuiContext = ImGui::CreateContext();
	}

	Gui::~Gui()
	{
		ImGui::DestroyContext(myGuiContext);
	}

	void Gui::Update()
	{

	}
}
