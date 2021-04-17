#include "VulkanCamera.h"

#include "VulkanHelpers.h"
#include "VulkanRenderer.h"

#include "Input.h"

namespace Render
{
namespace Vulkan
{
	void Camera::Update()
	{
		Input::InputManager* inputManager = Input::InputManager::GetInstance();

		double mouseX, mouseY;
		inputManager->PollMousePosition(mouseX, mouseY);

		static double prevMouseX = mouseX;
		static double prevMouseY = mouseY;

		double deltaMouseX = prevMouseX - mouseX;
		double deltaMouseY = prevMouseY - mouseY;

		if (inputManager->PollRawInput(Input::RawInput::MouseLeft) == Input::RawInputState::Pressed)
		{
			Rotate(glm::vec3(deltaMouseY, -deltaMouseX, 0.0f));
		}
		if (inputManager->PollRawInput(Input::RawInput::MouseMiddle) == Input::RawInputState::Pressed)
		{
			Translate(glm::vec3(-deltaMouseX * 0.01f, -deltaMouseY * 0.01f, 0.0f));
		}

		prevMouseX = mouseX;
		prevMouseY = mouseY;
		/*if (inputManager->PollRawInput(Input::RawInput::KeyW) == Input::RawInputState::Pressed)
			myPosition += myDirection * 0.1f;
		else if (inputManager->PollRawInput(Input::RawInput::KeyS) == Input::RawInputState::Pressed)
			myPosition -= myDirection * 0.1f;

		glm::mat4 view = glm::lookAt(myPosition, myPosition + myDirection, glm::vec3(0.0f, 0.0f, 1.0f));
		glm::mat4 proj = glm::perspective(glm::radians(45.0f), 800.f / 600.f, 0.1f, 10.0f);
		proj[1][1] *= -1; // adapt calculation for Vulkan*/
	}

	Camera::Camera()
	{
		Input::InputManager* inputManager = Input::InputManager::GetInstance();
		inputManager->AddScrollInputCallback([this](double aX, double aY) {
			(void)aX;
			Translate(glm::vec3(0.0f, 0.0f, (float)aY * 0.3f));
		});
	}

	Camera::~Camera()
	{
	}

	void Camera::SetPosition(const glm::vec3& aPosition)
	{
		myPosition = aPosition;
		UpdateViewMatrix();
	}

	void Camera::Translate(const glm::vec3& aPositionDelta)
	{
		myPosition += aPositionDelta;
		UpdateViewMatrix();
	}

	void Camera::SetRotation(const glm::vec3& aRotation)
	{
		myRotation = aRotation;
		UpdateViewMatrix();
	}

	void Camera::Rotate(const glm::vec3& aRotationDelta)
	{
		myRotation += aRotationDelta;
		UpdateViewMatrix();
	}

	void Camera::GetPosition(glm::vec3& anOutPosition) const
	{
		anOutPosition = myPosition;
	}

	void Camera::GetViewMatrix(glm::mat4& anOutMatrix) const
	{
		anOutMatrix = myView;
	}

	void Camera::SetPerspective(float anAspectRatio, float aFov, float aZNear, float aZFar)
	{
		myAspectRatio = anAspectRatio;
		myFov = aFov;
		myZNear = aZNear;
		myZFar = aZFar;
		UpdatePerspectiveMatrix();
	}

	void Camera::GetPerspectiveMatrix(glm::mat4& anOutMatrix) const
	{
		anOutMatrix = myPerspective;
	}

	void Camera::UpdateViewMatrix()
	{
		glm::mat4 rotationMatrix = glm::mat4(1.0f);
		rotationMatrix = glm::rotate(rotationMatrix, glm::radians(-myRotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
		rotationMatrix = glm::rotate(rotationMatrix, glm::radians(myRotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
		rotationMatrix = glm::rotate(rotationMatrix, glm::radians(myRotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

		glm::vec3 translation = myPosition;
		translation.y *= -1.0f;
		glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), translation);

		myView = translationMatrix * rotationMatrix;
	}

	void Camera::UpdatePerspectiveMatrix()
	{
		myPerspective = glm::perspective(glm::radians(myFov), myAspectRatio, myZNear, myZFar);
		myPerspective[1][1] *= -1.0f;
	}
}
}
