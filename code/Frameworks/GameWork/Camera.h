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
		void SetAspectRatio(float anAspectRatio) { myAspectRatio = anAspectRatio; }
		void SetFov(float aFov) { myFov = aFov; }
		void SetNearFar(float aZNear, float aZFar) { myZNear = aZNear; myZFar = aZFar; }

	private:
		float myAspectRatio = 1.0f;
		float myFov = 45.0f;
		float myZNear = 0.1f;
		float myZFar = 256.0f;

		uint myScrollCallbackId = UINT_MAX;

		glm::vec3 myPosition = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::vec3 myDirection = glm::vec3(0.0f, 0.0f, 1.0f);
		glm::vec3 myUp = glm::vec3(0.0f, 1.0f, 0.0f);
		glm::vec3 myLeft = glm::vec3(1.0f, 0.0f, 0.0f);
	};
}
