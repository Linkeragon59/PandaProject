#pragma once
#include "GameCore_Module.h"

namespace GameCore
{
	class EntityHandle;
}

class RhythmShooterModule : public GameCore::Module
{
DECLARE_GAMECORE_MODULE(RhythmShooterModule, "RhythmShooter")

protected:
	void OnRegister() override;
	void OnUnregister() override;
	void OnUpdate(GameCore::Module::UpdateType aType) override;

private:
	GameCore::Camera* myCamera = nullptr;
	GameCore::EntityHandle mySimpleGeometryTest;
	GameCore::EntityHandle myTestModel;
	GameCore::EntityHandle myTestAnimatedModel;
};
