#include "GameCore.h"
#include "PandaModule.h"

int main()
{
	if (!GameCore::GameCore::Create())
		return EXIT_FAILURE;

	PandaModule::Register();

	GameCore::GameCore::GetInstance()->Run();

	PandaModule::Unregister();
	
	GameCore::GameCore::Destroy();

	return EXIT_SUCCESS;
}
