#pragma once

#include <atomic>

namespace Time
{
	class TimeManager
	{
	public:
		static void Create();
		static void Destroy();
		static TimeManager* GetInstance() { return ourInstance; }

		void NextFrame();
		uint GetFrameCounter() const { return myFrameCounter; }

	private:
		static TimeManager* ourInstance;

		std::atomic<uint> myFrameCounter = 0;
	};
}
