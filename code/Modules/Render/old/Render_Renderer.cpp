#include "Render_Renderer.h"

#include "Render_DeferredRenderer.h"
#include "Render_EditorRenderer.h"

namespace Render
{
	Renderer::Renderer(Type aType)
		: myType(aType)
	{
		switch (aType)
		{
		case Type::Deferred:
			myImpl = new DeferredRenderer();
			break;
		case Type::Editor:
			myImpl = new EditorRenderer();
			break;
		default:
			Assert("Unsupported Renderer Type");
			break;
		}
	}

	Renderer::~Renderer()
	{
		delete myImpl;
	}

	void Renderer::SetViewProj(const glm::mat4& aView, const glm::mat4& aProjection)
	{
		myImpl->SetViewProj(aView, aProjection);
	}

	void Renderer::AddLight(const PointLight& aPointLight)
	{
		myImpl->AddLight(aPointLight);
	}

	void Renderer::DrawModel(Handle aModelHandle, const ModelData& someData, DrawType aDrawType /*= DrawType::Default*/)
	{
		myImpl->DrawModel(aModelHandle, someData, aDrawType);
	}

	void Renderer::DrawGui(Handle aGuiHandle)
	{
		myImpl->DrawGui(aGuiHandle);
	}
}
