#include "GameCore.h"
#include "Editor_EditorModule.h"
#include "RhythmShooterModule.h"

int main()
{
	InitMemoryLeaksDetection();

	if (!GameCore::GameCore::Create())
		return EXIT_FAILURE;

	Editor::EditorModule::Register();
	RhythmShooterModule::Register();
	
	GameCore::GameCore::GetInstance()->Run();

	RhythmShooterModule::Unregister();
	Editor::EditorModule::Unregister();

	GameCore::GameCore::Destroy();

	return EXIT_SUCCESS;
}
