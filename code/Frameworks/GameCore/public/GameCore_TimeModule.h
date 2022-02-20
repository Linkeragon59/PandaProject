#pragma once
#include "GameCore_Module.h"

#include <atomic>
#include <chrono>

namespace GameCore
{
	class TimeModule : public Module
	{
	DECLARE_GAMECORE_MODULE(TimeModule, "Time")

	protected:
		void OnRegister() override;
		void OnUpdate(GameCore::Module::UpdateType aType) override;

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
		std::chrono::nanoseconds myTimeNs;
		std::chrono::nanoseconds myDeltaTimeNs;
		std::chrono::duration<float> myTime;
		std::chrono::duration<float> myDeltaTime;

		std::chrono::high_resolution_clock::time_point myStartTime;
		std::chrono::high_resolution_clock::time_point myCurrentTime;

		std::atomic<uint> myFrameCounter = 0;
	};
}
