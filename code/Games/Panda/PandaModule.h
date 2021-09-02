#pragma once
#include "GameWork_Module.h"

class PandaModule : public GameWork::Module
{
DECLARE_GAMEWORK_MODULE(PandaModule, "Panda")

protected:
	void OnRegister() override;
	void OnUpdate() override;
	void OnUnregister() override;
};
