#pragma once

#include "Entity.h"

namespace Render
{
	class Renderer;
}

namespace GameWork
{
	class Camera : public Entity
	{
	public:
		Camera();
		~Camera();

		void Update() override;
		void Bind(Render::Renderer* aRenderer);

		const glm::mat4& GetViewMatrix() const { return myView; }

		void SetPerspective(float anAspectRatio, float aFov, float aZNear, float aZFar);
		const glm::mat4& GetPerspectiveMatrix() const { return myPerspective; }

	protected:
		void OnPositionChanged();
		void OnRotationChanged();

	private:
		void UpdateViewMatrix();
		void UpdatePerspectiveMatrix();

		float myAspectRatio = 1.0f;
		float myFov = 60.0f;
		float myZNear = 0.1f;
		float myZFar = 256.0f;

		glm::mat4 myView = glm::mat4(1.0f);
		glm::mat4 myPerspective = glm::mat4(1.0f);

		uint myScrollCallbackId = UINT_MAX;
	};
}
