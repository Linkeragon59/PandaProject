#include "Camera.h"

#include "RenderRenderer.h"
#include "Input.h"

namespace GameWork
{
	namespace
	{
		float locSpeed = 0.01f;
		float locSensitivity = 0.1f;
	}

	Camera::Camera()
	{
		Input::InputManager* inputManager = Input::InputManager::GetInstance();
		myScrollCallbackId = inputManager->AddScrollCallback([this](double aX, double aY) {
			(void)aX;
			myFov -= (float)aY;
			if (myFov < 1.0f)
				myFov = 1.0f;
			if (myFov > 90.0f)
				myFov = 90.0f;
		});
	}

	Camera::~Camera()
	{
		Input::InputManager* inputManager = Input::InputManager::GetInstance();
		inputManager->RemoveScrollCallback(myScrollCallbackId);
	}

	void Camera::Update()
	{
		myRight = glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), myDirection);
		myUp = glm::cross(myDirection, myRight);

		Input::InputManager* inputManager = Input::InputManager::GetInstance();

		double mouseX, mouseY;
		inputManager->PollMousePosition(mouseX, mouseY);

		static double prevMouseX = mouseX;
		static double prevMouseY = mouseY;

		double deltaMouseX = locSensitivity * (prevMouseX - mouseX);
		double deltaMouseY = locSensitivity * (prevMouseY - mouseY);

		prevMouseX = mouseX;
		prevMouseY = mouseY;

		if (inputManager->PollRawInput(Input::RawInput::MouseLeft) == Input::RawInputState::Pressed)
		{
			static float pitch = 0.0f;
			static float yaw = -90.0f;

			pitch += (float)deltaMouseY;
			if (pitch > 89.0f)
				pitch = 89.0f;
			if (pitch < -89.0f)
				pitch = -89.0f;
			yaw += (float)-deltaMouseX;

			myDirection.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
			myDirection.y = sin(glm::radians(pitch));
			myDirection.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
			myDirection = glm::normalize(myDirection);
		}
		if (inputManager->PollRawInput(Input::RawInput::MouseMiddle) == Input::RawInputState::Pressed)
		{
			myPosition += -0.1f * ((float)deltaMouseX * myRight + (float)deltaMouseY * myUp);
		}
		if (inputManager->PollRawInput(Input::RawInput::KeyW) == Input::RawInputState::Pressed)
		{
			myPosition += locSpeed * myDirection;
		}
		if (inputManager->PollRawInput(Input::RawInput::KeyS) == Input::RawInputState::Pressed)
		{
			myPosition -= locSpeed * myDirection;
		}
		if (inputManager->PollRawInput(Input::RawInput::KeyA) == Input::RawInputState::Pressed)
		{
			myPosition += locSpeed * myRight;
		}
		if (inputManager->PollRawInput(Input::RawInput::KeyD) == Input::RawInputState::Pressed)
		{
			myPosition -= locSpeed * myRight;
		}
	}

	void Camera::Bind(Render::Renderer* aRenderer)
	{
		glm::mat4 view = glm::lookAt(myPosition, myPosition + myDirection, myUp);
		glm::mat4 perspective = glm::perspective(glm::radians(myFov), myAspectRatio, myZNear, myZFar);
		perspective[1][1] *= -1;
		aRenderer->SetViewProj(view, perspective);
	}

	void Camera::SetPerspective(float anAspectRatio, float aFov, float aZNear, float aZFar)
	{
		myAspectRatio = anAspectRatio;
		myFov = aFov;
		myZNear = aZNear;
		myZFar = aZFar;
	}
}
