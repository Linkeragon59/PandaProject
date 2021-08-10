#include <stdexcept>
#include <iostream>

#include "Types.h"

#include "GameWork.h"
#include "PandaModule.h"

int main()
{
	if (!GameWork::GameWork::Create())
		return EXIT_FAILURE;

	PandaModule::Register();

	GameWork::GameWork::GetInstance()->Run();

	PandaModule::Unregister();
	
	GameWork::GameWork::Destroy();

	return EXIT_SUCCESS;
}