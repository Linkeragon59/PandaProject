#include "Render_GuiContainer.h"

#include "Render_Gui.h"

namespace Render
{
	GuiContainer::~GuiContainer()
	{
		for (uint i = 0; i < (uint)myGuis.size(); ++i)
		{
			if (myGuis[i])
				delete myGuis[i];
		}
	}

	Handle GuiContainer::AddGui(ImGuiContext* aGuiContext)
	{
		uint index = (uint)myGuis.size();
		for (uint i = 0; i < (uint)myGuis.size(); ++i)
		{
			if (!myGuis[i])
			{
				index = i;
				break;
			}
		}
		if (index >= (uint)myGuis.size())
			myGuis.push_back(nullptr);

		myGuis[index] = new Gui(aGuiContext);
		return Handle(index);
	}

	void GuiContainer::RemoveGui(Handle aGuiHandle)
	{
		Assert(aGuiHandle < (uint)myGuis.size());
		if (myGuis[aGuiHandle])
		{
			delete myGuis[aGuiHandle];
			myGuis[aGuiHandle] = nullptr;
		}
	}

	void GuiContainer::UpdateGui(Handle aGuiHandle)
	{
		Assert(aGuiHandle < (uint)myGuis.size());
		if (myGuis[aGuiHandle])
		{
			myGuis[aGuiHandle]->Update();
		}
	}

	Gui* GuiContainer::GetGui(Handle aGuiHandle) const
	{
		Assert(aGuiHandle < (uint)myGuis.size());
		return myGuis[aGuiHandle];
	}
}
