#include "glTFProp.h"

#include "RenderModel.h"

namespace GameWork
{
	glTFProp::glTFProp(std::string aFileName)
		: Prop()
	{
		myModelData = new Render::glTFModelData;
		static_cast<Render::glTFModelData*>(myModelData)->myFilename = aFileName;
	}
}
