#include "DummyGameObject.h"

#include <iostream>

int DummyGameObject::ourVarLimit = 100;

namespace
{
	const char* locDummyGameObjectDescription = "a dummy game object";
}

DummyGameObject::DummyGameObject()
{
	std::cout << locDummyGameObjectDescription << " instantiated\n";

	// Try to produce a warning
	//size_t a = 0;
	//myVar = a;
}

DummyGameObject::~DummyGameObject()
{
	std::cout << locDummyGameObjectDescription << " destroyed\n";
}

bool DummyGameObject::TrySetVar(int aNewValue)
{
	if (aNewValue > ourVarLimit)
		return false;

	myVar = aNewValue;
	return true;
}

bool DummyGameObject::TrySetVar(std::vector<int> somePossibleNewValue)
{
	for (int newVal : somePossibleNewValue)
	{
		if (TrySetVar(newVal))
			return true;
	}

	return false;
}
