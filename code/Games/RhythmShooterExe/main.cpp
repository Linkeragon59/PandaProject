#include <stdexcept>
#include <iostream>

#include "Types.h"

#include "GameWork.h"
#include "RhythmShooterModule.h"

int main()
{
	if (!GameWork::GameWork::Create())
		return EXIT_FAILURE;

	RhythmShooterModule::Register();

	GameWork::GameWork::GetInstance()->Run();

	RhythmShooterModule::Unregister();
	
	GameWork::GameWork::Destroy();

	return EXIT_SUCCESS;
}