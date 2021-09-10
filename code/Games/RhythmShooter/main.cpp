#include "GameWork.h"
#include "Editor_EditorModule.h"
#include "RhythmShooterModule.h"

int main()
{
	if (!GameWork::GameWork::Create())
		return EXIT_FAILURE;

	Editor::EditorModule::Register();
	RhythmShooterModule::Register();

	GameWork::GameWork::GetInstance()->Run();

	RhythmShooterModule::Unregister();
	Editor::EditorModule::Unregister();

	GameWork::GameWork::Destroy();

	return EXIT_SUCCESS;
}
