#pragma once

#include "Prop.h"

namespace GameWork
{
	class glTFProp : public Prop
	{
	public:
		void SetModelFilename(const std::string& aFilename);

	private:
		Render::ModelData* CreateModelData(const Render::ModelData& someData) override;
	};
}
