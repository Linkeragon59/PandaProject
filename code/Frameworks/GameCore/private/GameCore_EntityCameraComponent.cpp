#include "GameCore_EntityCameraComponent.h"

#include "GameCore_InputModule.h"
#include "GameCore_WindowModule.h"

namespace GameCore
{
	namespace
	{
		float locSpeed = 0.01f;
		float locSensitivity = 0.1f;
	}

	EntityCameraComponent::EntityCameraComponent()
	{
		InputModule* inputModule = InputModule::GetInstance();
		myScrollCallbackId = inputModule->AddScrollCallback([this](double aX, double aY) {
			(void)aX;
			myFov -= (float)aY;
			if (myFov < 1.0f)
				myFov = 1.0f;
			if (myFov > 90.0f)
				myFov = 90.0f;
			}, WindowModule::GetInstance()->GetMainWindow());
	}

	EntityCameraComponent::~EntityCameraComponent()
	{
		InputModule* inputModule = InputModule::GetInstance();
		inputModule->RemoveScrollCallback(myScrollCallbackId);
	}

	void EntityCameraComponent::Update()
	{
		myLeft = glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), myDirection);
		myUp = glm::cross(myDirection, myLeft);

		InputModule* inputModule = InputModule::GetInstance();

		double mouseX, mouseY;
		inputModule->PollMousePosition(mouseX, mouseY);

		static double prevMouseX = mouseX;
		static double prevMouseY = mouseY;

		double deltaMouseX = locSensitivity * (prevMouseX - mouseX);
		double deltaMouseY = locSensitivity * (prevMouseY - mouseY);

		prevMouseX = mouseX;
		prevMouseY = mouseY;

		if (inputModule->PollMouseInput(Input::MouseLeft) == Input::Status::Pressed)
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
		if (inputModule->PollMouseInput(Input::MouseMiddle) == Input::Status::Pressed)
		{
			myPosition += -0.1f * ((float)deltaMouseX * myLeft + (float)deltaMouseY * myUp);
		}
		if (inputModule->PollKeyInput(Input::KeyW) == Input::Status::Pressed)
		{
			myPosition += locSpeed * myDirection;
		}
		if (inputModule->PollKeyInput(Input::KeyS) == Input::Status::Pressed)
		{
			myPosition -= locSpeed * myDirection;
		}
		if (inputModule->PollKeyInput(Input::KeyA) == Input::Status::Pressed)
		{
			myPosition += locSpeed * myLeft;
		}
		if (inputModule->PollKeyInput(Input::KeyD) == Input::Status::Pressed)
		{
			myPosition -= locSpeed * myLeft;
		}
	}
}
