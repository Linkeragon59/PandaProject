#include "PandaModule.h"

#include "DummyGameObject.h"
#include "Perlin.h"
#include "Map.h"

#include "Base_Input.h"

DEFINE_GAMECORE_MODULE(PandaModule);

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

void PandaModule::OnUpdate(GameCore::Module::UpdateType /*aType*/)
{
	/*if (aType == GameCore::Module::UpdateType::MainUpdate)
	{
		Input::InputManager* inputManager = Input::InputManager::GetInstance();
		Input::RawInputState mouseLeftState = inputManager->PollRawInput(Input::RawInput::MouseLeft);
		if (mouseLeftState == Input::RawInputState::Pressed)
		{
			// Test Perlin and Map classes
			Map map;
			map.Print();
		}
	}*/
}

void PandaModule::OnUnregister()
{
}
