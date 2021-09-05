#pragma once

#include "Base_SharedPtr.h"

namespace Render
{
	// Used to delay the destruction of resources that are may still be used during the next few frames
	class RenderResource : public SharedResource
	{
	public:
		static void EnableDeleteQueue(bool aEnable);
		void Release(); // overload
	};
}
