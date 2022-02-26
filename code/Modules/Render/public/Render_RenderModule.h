#pragma once

#include "GameCore_Module.h"

#include "GameCore_Entity.h"
#include "Render_EntityRenderComponent.h"

struct GLFWwindow;

namespace Render
{
	enum class RendererType
	{
		Deferred,
		Count
	};

	class RenderCore;

	class RenderModule : public GameCore::Module
	{
	DECLARE_GAMECORE_MODULE(RenderModule, "Render")

	protected:
		void OnRegister() override;
		void OnUnregister() override;
		
		void OnInitialize() override;
		void OnFinalize() override;

		void OnUpdate(GameCore::Module::UpdateType aType) override;

	public:
		RenderCore* GetRenderCore() const { return myRenderCore; }

		void RegisterWindow(GLFWwindow* aWindow, RendererType aType);
		void UnregisterWindow(GLFWwindow* aWindow);

	private:
		RenderCore* myRenderCore;
	};
}
