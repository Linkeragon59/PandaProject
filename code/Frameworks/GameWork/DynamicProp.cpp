#include "DynamicProp.h"

namespace GameWork
{

	DynamicProp::DynamicProp()
	{
		myModelData = new Render::DynamicModelData;
	}

	void DynamicProp::SetGeometry(const std::vector<Render::DynamicModelData::Vertex>& someVertices, const std::vector<uint>& someIndices)
	{
		static_cast<Render::DynamicModelData*>(myModelData)->myVertices = someVertices;
		static_cast<Render::DynamicModelData*>(myModelData)->myIndices = someIndices;
	}

	void DynamicProp::SetTexture(const std::string& aTextureFileName)
	{
		static_cast<Render::DynamicModelData*>(myModelData)->myTextureFilename = aTextureFileName;
	}
}
