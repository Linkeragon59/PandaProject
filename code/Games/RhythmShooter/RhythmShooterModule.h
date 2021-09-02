#pragma once
#include "GameWork_Module.h"

namespace GameWork
{
	class Camera;
	class Prop;
}

class RhythmShooterModule : public GameWork::Module
{
DECLARE_GAMEWORK_MODULE(RhythmShooterModule, "RhythmShooter")

protected:
	void OnRegister() override;
	void OnUnregister() override;
	void OnUpdate() override;

private:
	GameWork::Camera* myCamera = nullptr;
	GameWork::Prop* mySimpleGeometryTest = nullptr;
	GameWork::Prop* myTestModel = nullptr;
	GameWork::Prop* myTestAnimatedModel = nullptr;
};
