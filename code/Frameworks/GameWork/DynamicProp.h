#pragma once

#include "Prop.h"
#include "RenderModel.h"

namespace GameWork
{
	class DynamicProp : public Prop
	{
	public:
		DynamicProp();
		~DynamicProp() {}

		void SetGeometry(const std::vector<Render::DynamicModelData::Vertex>& someVertices, const std::vector<uint>& someIndices);
		void SetTexture(const std::string& aTextureFileName);
	};
}
