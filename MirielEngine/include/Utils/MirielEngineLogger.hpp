#pragma once

#include "CustomErrors/MirielEngineErrors.hpp"
#include "Utils/ThreadsafeQueue.hpp"

#include <memory>
#include <mutex>
#include <chrono>
#include <thread>
#include <string>

namespace MirielEngine::Utils {
	struct LoggingMessage {
		std::string msg;
		std::chrono::system_clock::time_point time;
	};

	void LoggingThreadFunction(std::shared_ptr<DataStructures::ThreadsafeQueue<std::unique_ptr<LoggingMessage>>> msgBuffer, const std::string& filename);
	std::string createLoggingFileName();

	class Logger {
		private:
			std::shared_ptr<DataStructures::ThreadsafeQueue<std::unique_ptr<LoggingMessage>>> msgQueue;

			static Logger* instance;
			const std::string currentLog = createLoggingFileName();
			static std::mutex mtx;

			bool programRunning;
			std::thread loggingThread;

			Logger();
			Logger(const Logger& obj) = delete;
		public:
			~Logger();
			void log(const std::string& msg);
			static Logger* getInstance();
			std::string getCurrentLog();
			bool getProgramRunning();
			void cleanup();
	};

	inline Logger * const GlobalLogger = Logger::getInstance();
}