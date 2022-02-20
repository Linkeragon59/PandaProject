#include "GameCore_Facade.h"
#include "PandaModule.h"

int main()
{
	if (!GameCore::Facade::Create())
		return EXIT_FAILURE;

	PandaModule::Register();

	GameCore::Facade::GetInstance()->Run();

	PandaModule::Unregister();
	
	GameCore::Facade::Destroy();

	return EXIT_SUCCESS;
}
