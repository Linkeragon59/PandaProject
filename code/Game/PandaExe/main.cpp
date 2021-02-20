#include <stdexcept>
#include <iostream>

#include "RenderFacade.h"
#include "DummyGameObject.h"
#include "Perlin.h"

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

	// This doesn't work, I get "undefined reference to Perlin::Perlin"
	Perlin field;
	std::vector perm = field.GetPerm();
	std::cout << "The random permutation is: ";
	for(int i = 0; i < 256; i++)
		std::cout << perm.at(i) << ", ";
	std::cout << std::endl;
	std::cout << "Perlin field evaluated at (0.5, 0.5) is " << field.NoisePt(0.5, 0.5) << std::endl;

	return EXIT_SUCCESS;
}