#pragma once

#include "Prop.h"

namespace GameWork
{
	class glTFProp : public Prop
	{
	public:
		glTFProp();
		~glTFProp() {}

		void SetModelFilename(const std::string& aFilename);
	};
}
