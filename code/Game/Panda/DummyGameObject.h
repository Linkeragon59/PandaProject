#pragma once

#include <vector>

class DummyGameObject
{
public:
	DummyGameObject();
	~DummyGameObject();

	int GetVar() const { return myVar; }
	bool TrySetVar(int aNewValue);
	bool TrySetVar(std::vector<int> somePossibleNewValue);
	
private:
	int myVar = 0;
	static int ourVarLimit;
};
