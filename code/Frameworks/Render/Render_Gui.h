#pragma once

namespace Render
{
	class Gui
	{
	public:
		virtual ~Gui() {}
		virtual void Update() = 0;
	};
}
