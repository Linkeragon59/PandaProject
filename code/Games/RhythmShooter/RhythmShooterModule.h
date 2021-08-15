#pragma once
#include "Module.h"

namespace GameWork
{
	class Camera;
	class glTFProp;
	class DynamicProp;
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
	GameWork::DynamicProp* myShooterPlane = nullptr;
	GameWork::glTFProp* myTestModel = nullptr;
	GameWork::glTFProp* myTestAnimatedModel = nullptr;
#if DEBUG_BUILD
	GameWork::DynamicProp* myVectorBase = nullptr;
#endif
};
