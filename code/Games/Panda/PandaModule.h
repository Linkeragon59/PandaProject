#pragma once
#include "GameCore_Module.h"

class PandaModule : public GameCore::Module
{
DECLARE_GAMECORE_MODULE(PandaModule, "Panda")

protected:
	void OnRegister() override;
	void OnUpdate(GameCore::Module::UpdateType aType) override;
	void OnUnregister() override;
};
