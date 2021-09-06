#include "GameWork_glTFProp.h"

#include "Render_ModelData.h"

namespace GameWork
{
	void glTFProp::SetModelFilename(const std::string& aFilename)
	{
		static_cast<Render::glTFModelData*>(myModelData)->myFilename = aFilename;
	}

	Render::ModelData* glTFProp::CreateModelData(const Render::ModelData& someData)
	{
		Assert(someData.GetType() == Render::ModelData::Type::glTF);
		return new Render::glTFModelData(static_cast<const Render::glTFModelData&>(someData));
	}
}
