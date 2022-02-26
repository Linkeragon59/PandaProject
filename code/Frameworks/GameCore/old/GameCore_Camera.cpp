#include "GameCore_Camera.h"

#include "Render_Renderer.h"
#include "GameCore_Input.h"
#include "GameCore_Window.h"

namespace GameCore
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
		}, Window::WindowManager::GetInstance()->GetMainWindow());
	}

	Camera::~Camera()
	{
		Input::InputManager* inputManager = Input::InputManager::GetInstance();
		inputManager->RemoveScrollCallback(myScrollCallbackId);
	}

	void Camera::Update()
	{
		myLeft = glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), myDirection);
		myUp = glm::cross(myDirection, myLeft);

		Input::InputManager* inputManager = Input::InputManager::GetInstance();

		double mouseX, mouseY;
		inputManager->PollMousePosition(mouseX, mouseY);

		static double prevMouseX = mouseX;
		static double prevMouseY = mouseY;

		double deltaMouseX = locSensitivity * (prevMouseX - mouseX);
		double deltaMouseY = locSensitivity * (prevMouseY - mouseY);

		prevMouseX = mouseX;
		prevMouseY = mouseY;

		if (inputManager->PollMouseInput(Input::MouseLeft) == Input::Status::Pressed)
		{
			static float pitch = 0.0f;
			static float yaw = -90.0f;

			pitch += (float)deltaMouseY;
			if (pitch > 89.0f)
				pitch = 89.0f;
			if (pitch < -89.0f)
				pitch = -89.0f;
			yaw -= (float)deltaMouseX;

			myDirection.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
			myDirection.y = sin(glm::radians(pitch));
			myDirection.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
			myDirection = glm::normalize(myDirection);
		}
		if (inputManager->PollMouseInput(Input::MouseMiddle) == Input::Status::Pressed)
		{
			myPosition += -0.1f * ((float)deltaMouseX * myLeft + (float)deltaMouseY * myUp);
		}
		if (inputManager->PollKeyInput(Input::KeyW) == Input::Status::Pressed)
		{
			myPosition += locSpeed * myDirection;
		}
		if (inputManager->PollKeyInput(Input::KeyS) == Input::Status::Pressed)
		{
			myPosition -= locSpeed * myDirection;
		}
		if (inputManager->PollKeyInput(Input::KeyA) == Input::Status::Pressed)
		{
			myPosition += locSpeed * myLeft;
		}
		if (inputManager->PollKeyInput(Input::KeyD) == Input::Status::Pressed)
		{
			myPosition -= locSpeed * myLeft;
		}
	}

	void Camera::Bind(Render::Renderer* aRenderer)
	{
		glm::mat4 view = glm::lookAt(myPosition, myPosition + myDirection, myUp);
		glm::mat4 perspective = glm::perspective(glm::radians(myFov), myAspectRatio, myZNear, myZFar);
		perspective[1][1] *= -1;
		aRenderer->SetViewProj(view, perspective);
	}
}
