#pragma once
#include "GameCore_Module.h"
#include "GameCore_Entity.h"

class RhythmShooterModule : public GameCore::Module
{
DECLARE_GAMECORE_MODULE(RhythmShooterModule, "RhythmShooter")

protected:
	void OnRegister() override;
	void OnUnregister() override;
	void OnUpdate(GameCore::Module::UpdateType aType) override;

private:
	//GameCore::Camera* myCamera = nullptr;
	GameCore::Entity mySimpleGeometryTest;
	GameCore::Entity myTestModel;
	GameCore::Entity myTestAnimatedModel;
};
