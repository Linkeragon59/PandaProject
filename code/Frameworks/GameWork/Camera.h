#pragma once

namespace Render
{
	class Renderer;
}

namespace GameWork
{
	class Camera
	{
	public:
		Camera();
		~Camera();

		void Update();
		void Bind(Render::Renderer* aRenderer);

		void SetPosition(const glm::vec3& aPosition) { myPosition = aPosition; }
		void SetDirection(const glm::vec3& aDirection) { myDirection = aDirection; }
		void SetPerspective(float anAspectRatio, float aFov, float aZNear, float aZFar);

	private:
		float myAspectRatio = 1.0f;
		float myFov = 60.0f;
		float myZNear = 0.1f;
		float myZFar = 256.0f;

		uint myScrollCallbackId = UINT_MAX;

		glm::vec3 myPosition = glm::vec3(0.0f, 0.0f, 2.0f);
		glm::vec3 myDirection = glm::vec3(0.0f, 0.0f, 1.0f);
		glm::vec3 myUp = glm::vec3(0.0f, 1.0f, 0.0f);
		glm::vec3 myRight = glm::vec3(1.0f, 0.0f, 0.0f);
	};
}
