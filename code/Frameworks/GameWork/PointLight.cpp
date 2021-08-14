#include "PointLight.h"

#include "RenderRenderer.h"

namespace GameWork
{
	void PointLight::Update()
	{
		myLightData.myPosition = GetMatrix()[3];
	}

	void PointLight::Bind(Render::Renderer* aRenderer)
	{
		aRenderer->AddLight(myLightData);
	}
}
