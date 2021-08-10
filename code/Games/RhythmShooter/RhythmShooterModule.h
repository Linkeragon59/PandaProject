#pragma once
#include "Module.h"

class RhythmShooterModule : public GameWork::Module
{
DECLARE_GAMEWORK_MODULE(RhythmShooterModule, "RhythmShooter")

protected:
	void OnRegister() override;
	void OnUpdate() override;
	void OnUnregister() override;
};
