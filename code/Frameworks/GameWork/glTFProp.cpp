#include "glTFProp.h"

#include "RenderModel.h"

namespace GameWork
{
	glTFProp::glTFProp()
	{
		myModelData = new Render::glTFModelData;
		
	}

	void glTFProp::SetModelFilename(const std::string& aFilename)
	{
		static_cast<Render::glTFModelData*>(myModelData)->myFilename = aFilename;
	}

}
