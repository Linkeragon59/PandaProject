#include "PandaModule.h"

#include "DummyGameObject.h"
#include "Perlin.h"
#include "Map.h"

#include "Input.h"

PandaModule* PandaModule::ourInstance = nullptr;

void PandaModule::OnRegister()
{
	DummyGameObject* object = new DummyGameObject();

	int var = (object->GetVar() + 1) * 2;
	while (object->TrySetVar(var))
	{
		var = (object->GetVar() + 1) * 2;
	}

	delete object;
}

void PandaModule::OnUpdate()
{
	Input::InputManager* inputManager = Input::InputManager::GetInstance();
	Input::RawInputState mouseLeftState = inputManager->PollRawInput(Input::RawInput::MouseLeft);
	if (mouseLeftState == Input::RawInputState::Pressed)
	{
		// Test Perlin and Map classes
		Map map;
		map.Print();
	}
}

void PandaModule::OnUnregister()
{
}
