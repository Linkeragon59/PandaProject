#pragma once

#include "GameCore_Prop.h"

namespace GameCore
{
	class glTFProp : public Prop
	{
	public:
		void SetModelFilename(const std::string& aFilename);

	private:
		Render::ModelData* CreateModelData(const Render::ModelData& someData) override;
	};
}
