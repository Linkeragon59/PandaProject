#include "Render_Resource.h"

#include "GameCore_Thread.h"
#include "GameCore_Time.h"

namespace Render
{
	struct RenderResourceDeleteQueue
	{
		RenderResourceDeleteQueue()
		{
#if DEBUG_BUILD
			myThread.SetName("RenderResourceDeleteQueue");
#endif
		}

		void Enable(bool aEnable)
		{
			if (!myIsEnabled && aEnable)
			{
				myThread.Start([this]() {
					if (myResourcesToDelete.empty())
						return;

					std::lock_guard<std::mutex> lock(myMutex);
					ResourceToDelete* resource = &myResourcesToDelete.front();
					while (resource && Time::TimeManager::GetInstance()->GetFrameCounter() >= resource->myFrameToRelease)
					{
						delete resource->myResource;
						myResourcesToDelete.pop();
						resource = myResourcesToDelete.empty() ? nullptr : &myResourcesToDelete.front();
					}
				}, Thread::WorkerPriority::Low, 100);
			}
			else if (myIsEnabled && !aEnable)
			{
				myThread.StopAndWait();

				std::lock_guard<std::mutex> lock(myMutex);
				ResourceToDelete* resource = &myResourcesToDelete.front();
				while (resource)
				{
					delete resource->myResource;
					myResourcesToDelete.pop();
					resource = myResourcesToDelete.empty() ? nullptr : &myResourcesToDelete.front();
				}
			}
			myIsEnabled = aEnable;
		}

		void AddToDelete(RenderResource* aResource)
		{
			ResourceToDelete resource = { aResource, Time::TimeManager::GetInstance()->GetFrameCounter() + RenderModule::GetInstance()->GetMaxInFlightFramesCount() };
			
			std::lock_guard<std::mutex> lock(myMutex);
			myResourcesToDelete.push(resource);
		}

		Thread::WorkerThread myThread;
		bool myIsEnabled = false;

		struct ResourceToDelete 
		{
			RenderResource* myResource;
			uint myFrameToRelease;
		};
		std::mutex myMutex;
		std::queue<ResourceToDelete> myResourcesToDelete;
	};

	static RenderResourceDeleteQueue theDeleteQueue;

	void RenderResource::Release()
	{
		if (myRefCount.fetch_sub(1) == 1)
		{
			theDeleteQueue.AddToDelete(this);
		}
	}

	void RenderResource::EnableDeleteQueue(bool aEnable)
	{
		theDeleteQueue.Enable(aEnable);
	}
}
