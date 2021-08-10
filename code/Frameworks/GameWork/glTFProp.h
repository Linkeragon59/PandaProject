#pragma once

#include "Prop.h"

#include <string>

namespace GameWork
{
	class glTFProp : public Prop
	{
	public:
		glTFProp(std::string aFileName);
		~glTFProp() {}
	};
}
