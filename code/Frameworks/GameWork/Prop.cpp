#include "Prop.h"

namespace GameWork
{
	Prop::Prop(const char* aFilepath, const glm::vec3& aPosition, const glm::vec3& anOrientation, float aScale)
		: myFilepath(aFilepath)
		, myPosition(aPosition)
		, myOrientation(anOrientation)
		, myScale(aScale)
	{
	}

	Prop::~Prop()
	{
		if (myIsSpawned)
			Unspawn();
	}

	void Prop::Update()
	{

	}

	void Prop::Spawn()
	{

	}

	void Prop::Unspawn()
	{

	}
}
