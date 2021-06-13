#include "Thread.h"

#if WINDOWS_BUILD
#include <windows.h>
#elif LINUX_BUILD
#endif

namespace ThreadHelpers
{
	Worker::Worker(WorkerPool* aPool)
		: myPool(aPool)
	{
		myWorkerThread = std::thread(&Worker::RunJobs, this);

#if WINDOWS_BUILD

		int priority = THREAD_PRIORITY_NORMAL;
		switch (myPool->myWorkersPriority)
		{
		case WorkerPriority::High:
			priority = THREAD_PRIORITY_ABOVE_NORMAL;
			break;
		case WorkerPriority::Low:
			priority = THREAD_PRIORITY_BELOW_NORMAL;
			break;
		default:
			break;
		}
		SetThreadPriority(myWorkerThread.native_handle(), priority);

#if DEBUG_BUILD
		if (!myPool->myWorkersBaseName.empty())
		{
			std::wstring workerName = std::wstring(myPool->myWorkersBaseName.begin(), myPool->myWorkersBaseName.end());
			workerName += L" ";
			workerName += std::to_wstring(myPool->myWorkers.size());
			SetThreadDescription(myWorkerThread.native_handle(), workerName.c_str());
		}
#endif

#elif LINUX_BUILD
		// TODO
#endif
	}

	Worker::~Worker()
	{
		if (myWorkerThread.joinable())
		{
			WaitJobs();

			{
				std::unique_lock<std::mutex> lock(myJobQueueMutex);
				Assert(myJobQueue.empty(), "Jobs remaining in the queue!");
				myStopping = true;
			}

			// Tell the worker it should run (so it can end)
			myJobQueueCondition.notify_one();

			myWorkerThread.join();
		}
	}

	void Worker::AssignJob(std::function<void()> aJob)
	{
		{
			std::unique_lock<std::mutex> lock(myJobQueueMutex);
			myJobQueue.push(std::move(aJob));
		}

		// Tell the worker it has work to do
		myJobQueueCondition.notify_one();
	}

	void Worker::NotifyWaitingJobs()
	{
		// Tell the worker there is work waiting
		myJobQueueCondition.notify_one();
	}

	void Worker::WaitJobs()
	{
		std::unique_lock<std::mutex> lock(myJobQueueMutex);
		myJobQueueCondition.wait(lock, [this] { return !EvaluateWorkToDo(); });
	}

	void Worker::RunJobs()
	{
		while (true)
		{
			std::function<void()> nextJob;

			{
				std::unique_lock<std::mutex> lock(myJobQueueMutex);
				myJobQueueCondition.wait(lock, [this] { return EvaluateWorkToDo(); });
				if (myStopping)
				{
					Assert(myJobQueue.empty(), "Jobs remaining in the queue!");
					break;
				}
				nextJob = myJobQueue.front();
			}

			nextJob();

			{
				std::lock_guard<std::mutex> lock(myJobQueueMutex);
				myJobQueue.pop();
			}

			// In case a thread is waiting for jobs, tell it to re-evaluate the state of the queue
			myJobQueueCondition.notify_one();
		}
	}

	bool Worker::EvaluateWorkToDo()
	{
		if (myStopping)
			return true;

		if (!myJobQueue.empty())
			return true;

		if (myPool->AssignJob(this))
			return true;

		return false;
	}

	WorkerPool::WorkerPool(WorkerPriority aPriority /*= WorkerPriority::High*/)
		: myWorkersPriority(aPriority)
	{
	}

	void WorkerPool::SetWorkersCount(uint aCount /*= UINT_MAX*/)
	{
		// Releasing the workers will cause to wait
		myWorkers.clear();

		aCount = (std::min)(aCount, std::thread::hardware_concurrency());
		myWorkers.reserve(aCount);
		for (uint i = 0; i < aCount; ++i)
		{
			myWorkers.push_back(std::make_unique<Worker>(this));
		}
	}

	void WorkerPool::RequestJob(std::function<void()> aJob, uint aWorkIndex /*= UINT_MAX*/)
	{
		if (aWorkIndex < myWorkers.size())
		{
			myWorkers[aWorkIndex]->AssignJob(aJob);
			return;
		}

		// Put the job in a waiting queue, and the first worker that is done with its jobs will pick it.
		{
			std::unique_lock<std::mutex> lock(myWaitingJobQueueMutex);
			myWaitingJobQueue.push(std::move(aJob));
		}

		// Notify the workers there is work waiting.
		for (uint i = 0; i < myWorkers.size(); ++i)
		{
			myWorkers[i]->NotifyWaitingJobs();
		}
	}

	void WorkerPool::Wait()
	{
		// Must wait for waiting jobs too!!
		for (uint i = 0; i < myWorkers.size(); ++i)
		{
			myWorkers[i]->WaitJobs();
		}
	}

	bool WorkerPool::AssignJob(Worker* aWorker)
	{
		std::unique_lock<std::mutex> lock(myWaitingJobQueueMutex);

		if (myWaitingJobQueue.empty())
			return false;

		// This is called by the worker thread, and the worker mutex is already locked at this point
		aWorker->myJobQueue.push(myWaitingJobQueue.front());

		myWaitingJobQueue.pop();
		return true;
	}
}
