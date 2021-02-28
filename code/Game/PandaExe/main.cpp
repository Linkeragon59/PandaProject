#include <stdexcept>
#include <iostream>

#include "RenderFacade.h"
#include "DummyGameObject.h"
#include "Perlin.h"
#include "Map.h"
#include "Input.h"

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

	// Test Perlin and Map classes
	Map map;
	map.Print();

	// Test InputManager class
	Base::InputManager input;
	input.Create();
	input.PollInput(Base::Button1);

	return EXIT_SUCCESS;
}