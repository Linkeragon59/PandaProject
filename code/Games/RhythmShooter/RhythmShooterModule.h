#pragma once
#include "Module.h"

namespace GameWork
{
	class Camera;
	class glTFProp;
	class SimpleGeometryProp;
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
	GameWork::SimpleGeometryProp* mySimpleGeometryTest = nullptr;
	GameWork::glTFProp* myTestModel = nullptr;
	GameWork::glTFProp* myTestAnimatedModel = nullptr;
#if DEBUG_BUILD
	GameWork::SimpleGeometryProp* myVectorBase = nullptr;
#endif
};
