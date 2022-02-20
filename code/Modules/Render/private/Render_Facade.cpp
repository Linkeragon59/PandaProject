#include "Render_Facade.h"

#include "Render_ModelContainer.h"
#include "Render_GuiContainer.h"

namespace Render
{
	Handle AddModel(const ModelData& someData)
	{
		return RenderModule::GetInstance()->GetModelContainer()->AddModel(someData);
	}

	void RemoveModel(Handle aModelHandle)
	{
		RenderModule::GetInstance()->GetModelContainer()->RemoveModel(aModelHandle);
	}

	void UpdateModel(Handle aModelHandle, const ModelData& someData)
	{
		RenderModule::GetInstance()->GetModelContainer()->UpdateModel(aModelHandle, someData);
	}

	Handle AddGui(ImGuiContext* aGuiContext)
	{
		return RenderModule::GetInstance()->GetGuiContainer()->AddGui(aGuiContext);
	}

	void RemoveGui(Handle aGuiHandle)
	{
		RenderModule::GetInstance()->GetGuiContainer()->RemoveGui(aGuiHandle);
	}

	void UpdateGui(Handle aGuiHandle)
	{
		RenderModule::GetInstance()->GetGuiContainer()->UpdateGui(aGuiHandle);
	}
}
