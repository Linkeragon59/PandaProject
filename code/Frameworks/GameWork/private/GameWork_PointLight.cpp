#include "GameWork_PointLight.h"

#include "Render_Renderer.h"

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
