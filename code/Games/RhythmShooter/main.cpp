#include "GameCore_Facade.h"
#include "Render_RenderModule.h"
#include "Editor_EditorModule.h"
#include "RhythmShooterModule.h"

int main()
{
	InitMemoryLeaksDetection();

	if (!GameCore::Facade::Create())
		return EXIT_FAILURE;

	Render::RenderModule::Register();
	Editor::EditorModule::Register();
	RhythmShooterModule::Register();
	
	GameCore::Facade::GetInstance()->Run();

	RhythmShooterModule::Unregister();
	Editor::EditorModule::Unregister();
	Render::RenderModule::Unregister();

	GameCore::Facade::Destroy();

	return EXIT_SUCCESS;
}
