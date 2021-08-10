#include "PointLight.h"

#include "RenderRenderer.h"

namespace GameWork
{
	void PointLight::Bind(Render::Renderer* aRenderer)
	{
		aRenderer->AddLight(myLightData);
	}
}
