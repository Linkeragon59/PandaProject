#include <stdexcept>
#include <iostream>

#include "RenderFacade.h"
#include "DummyGameObject.h"

int main()
{
	DummyGameObject* object = new DummyGameObject();

	int var = (object->GetVar() + 1) * 2;
	while (object->TrySetVar(var))
	{
		var = (object->GetVar() + 1) * 2;
	}

	delete object;

	try
	{
		//Render::Facade::RunTriangleRenderer();
		Render::Facade::RunBasicRenderer();
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}