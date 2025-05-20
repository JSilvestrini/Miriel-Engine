#pragma once

#include <mutex>
#include <queue>

namespace MirielEngine::Utils::DataStructures {
	template <typename T>
	class ThreadsafeQueue {
		private:
			std::queue<T> _queue;
			std::mutex _mtx;
			std::condition_variable _cv;
		public:
			ThreadsafeQueue() = default;
			~ThreadsafeQueue() = default;
			ThreadsafeQueue(ThreadsafeQueue&&) noexcept = default;
			ThreadsafeQueue(const ThreadsafeQueue&) = default;
			ThreadsafeQueue& operator=(ThreadsafeQueue&&) noexcept = default;
			ThreadsafeQueue& operator=(const ThreadsafeQueue&) = default;

			void push(T&& item) {
				std::unique_lock<std::mutex> lock(_mtx);
				_queue.push(std::move(item));
				_cv.notify_one();
			}

			T front() {
				std::unique_lock<std::mutex> lock(_mtx);
				_cv.wait(lock, [this]{ return !_queue.empty(); });
				//T ret = std::move(_queue.front());
				return std::move(_queue.front());
			}

			void pop() {
				std::unique_lock<std::mutex> lock(_mtx);
				_cv.wait(lock, [this] { return !_queue.empty(); });
				_queue.pop();
			}

			size_t size() {
				std::unique_lock<std::mutex> lock(_mtx);
				return _queue.size();
			}

			bool empty() {
				std::unique_lock<std::mutex> lock(_mtx);
				return _queue.empty();
			}
	};
}