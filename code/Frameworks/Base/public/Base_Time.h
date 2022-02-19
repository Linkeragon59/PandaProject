#pragma once

#include <atomic>
#include <chrono>

namespace Time
{
	class TimeManager
	{
	public:
		static void Create();
		static void Destroy();
		static TimeManager* GetInstance() { return ourInstance; }

		void NextFrame();

		// Time since startup
		uint64 GetTimeNs() const { return myTimeNs.count(); }
		uint64 GetTimeMs() const { return std::chrono::duration_cast<std::chrono::milliseconds>(myTimeNs).count(); }
		float GetTime() const { return myTime.count(); }

		// Duration of last frame
		uint64 GetDeltaTimeNs() const { return myDeltaTimeNs.count(); }
		uint64 GetDeltaTimeMs() const { return std::chrono::duration_cast<std::chrono::milliseconds>(myDeltaTimeNs).count(); }
		float GetDeltaTime() const { return myDeltaTime.count(); }

		uint GetFrameCounter() const { return myFrameCounter; }

	private:
		static TimeManager* ourInstance;
		TimeManager();

		std::chrono::nanoseconds myTimeNs;
		std::chrono::nanoseconds myDeltaTimeNs;
		std::chrono::duration<float> myTime;
		std::chrono::duration<float> myDeltaTime;

		std::chrono::high_resolution_clock::time_point myStartTime;
		std::chrono::high_resolution_clock::time_point myCurrentTime;

		std::atomic<uint> myFrameCounter = 0;
	};
}
