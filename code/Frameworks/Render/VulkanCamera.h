#pragma once

#include "VulkanBuffer.h"

namespace Render
{
namespace Vulkan
{
	class Camera
	{
	public:
		Camera();
		~Camera();

		void Update();

		void SetPosition(const glm::vec3& aPosition);
		void Translate(const glm::vec3& aPositionDelta);
		void SetRotation(const glm::vec3& aRotation);
		void Rotate(const glm::vec3& aRotationDelta);
		void GetPosition(glm::vec3& anOutPosition) const;
		void GetViewMatrix(glm::mat4& anOutMatrix) const;

		void SetPerspective(float anAspectRatio, float aFov, float aZNear, float aZFar);
		void GetPerspectiveMatrix(glm::mat4& anOutMatrix) const;

	private:
		void UpdateViewMatrix();
		void UpdatePerspectiveMatrix();

		glm::vec3 myPosition = glm::vec3();
		glm::vec3 myRotation = glm::vec3();

		float myAspectRatio = 1.0f;
		float myFov = 60.0f;
		float myZNear = 0.1f;
		float myZFar = 256.0f;

		glm::mat4 myView;
		glm::mat4 myPerspective;
	};
}
}
