#include "Camera.h"

#include "RenderFacade.h"
#include "Input.h"

namespace GameWork
{
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
			Translate(glm::vec3(-deltaMouseX * 0.01f, deltaMouseY * 0.01f, 0.0f));
		}

		prevMouseX = mouseX;
		prevMouseY = mouseY;
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

	void Camera::SetPerspective(float anAspectRatio, float aFov, float aZNear, float aZFar)
	{
		myAspectRatio = anAspectRatio;
		myFov = aFov;
		myZNear = aZNear;
		myZFar = aZFar;
		UpdatePerspectiveMatrix();
	}

	void Camera::UpdateViewMatrix()
	{
		glm::mat4 rotationMatrix = glm::mat4(1.0f);
		rotationMatrix = glm::rotate(rotationMatrix, glm::radians(-myRotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
		rotationMatrix = glm::rotate(rotationMatrix, glm::radians(myRotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
		rotationMatrix = glm::rotate(rotationMatrix, glm::radians(myRotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

		glm::vec3 translation = myPosition;
		glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), translation);

		myView = translationMatrix * rotationMatrix;
		//myView = rotationMatrix * translationMatrix;
	}

	void Camera::UpdatePerspectiveMatrix()
	{
		myPerspective = glm::perspective(glm::radians(myFov), myAspectRatio, myZNear, myZFar);
		myPerspective[1][1] *= -1.0f; // Adapt for Vulkan
	}
}
