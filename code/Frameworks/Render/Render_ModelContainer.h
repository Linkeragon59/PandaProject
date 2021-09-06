#pragma once

#include "Render_Handle.h"

namespace Render
{
	class Model;

	class ModelContainer
	{
	public:
		~ModelContainer();

		Handle AddModel(const ModelData& someData);
		void RemoveModel(Handle aModelHandle);
		void UpdateModel(Handle aModelHandle, const ModelData& someData);

		Model* GetModel(Handle aModelHandle) const;

	private:
		std::vector<Model*> myModels; // TODO : store available slots in a list
	};
}
