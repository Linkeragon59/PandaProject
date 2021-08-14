#pragma once

#include "Entity.h"
#include "RenderLight.h"

namespace Render
{
	class Renderer;
}

namespace GameWork
{
	class PointLight : public Entity
	{
	public:
		void Update() override;
		void Bind(Render::Renderer* aRenderer);

		void SetColor(const glm::vec3& aColor) { myLightData.myColor = glm::vec4(aColor, myLightData.myColor.w); }
		glm::vec3 GetColor() const { return glm::vec3(myLightData.myColor); }

		void SetIntensity(float anIntensity) { myLightData.myColor.w = anIntensity; }
		float GetIntensity() const { return myLightData.myColor.w; }

	private:
		Render::PointLight myLightData;
	};
}
