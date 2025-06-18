#include "Utils/MirielEngineLogger.hpp"

#include <sstream>
#include <iomanip>
#include <ctime>
#include <filesystem>
#include <fstream>

namespace MirielEngine::Utils {
	void LoggingThreadFunction (std::shared_ptr<DataStructures::ThreadsafeQueue<std::unique_ptr<LoggingMessage>>> msgBuffer, const std::string& filename) {
		MirielEngine::Utils::GlobalLogger->log("Within Thread: Making Logging File.");
		std::ofstream file(filename, std::ios::out | std::ios::app | std::ios::binary);

		if (file.fail() || file.bad()) {
			std::ostringstream out;
			out << "Failed to Open File for Logging:\nThread ID: " << std::this_thread::get_id() << " Within " << __FILE__ << " at Line: " << __LINE__;
			out << "\nFile Fail: " << file.fail() << "\nFile Bad: " << file.bad() << "\nFile is Open: " << file.is_open();
			out << "\nFile Name: " << filename;
			throw MirielEngine::Errors::LoggingError(out.str().c_str());
		}

		auto LoggingLoop = [&](size_t minNum, size_t maxNum) {
			if (msgBuffer->size() < minNum) { return; }

			for (int i = 0; i < std::min(maxNum, msgBuffer->size()); i++) {
				std::unique_ptr<LoggingMessage> curr = std::move(msgBuffer->front());
				msgBuffer->pop();

				std::ostringstream out;
				std::time_t currTime = std::chrono::system_clock::to_time_t(curr->time);
				tm convertedCurrTime;
				localtime_s(&convertedCurrTime, &currTime);
				out << std::put_time(&convertedCurrTime, "%H:%M:%S: ") << curr->msg << "\n";

				file << out.str().c_str();
			}

			file.flush();
		};

		while (MirielEngine::Utils::GlobalLogger->getProgramRunning() || !msgBuffer->empty()) {
			LoggingLoop(1, 32);
			std::this_thread::sleep_for(std::chrono::milliseconds(20));
		}

		MirielEngine::Utils::GlobalLogger->log("Within Thread: Application Finished, Finishing Logging.");
		size_t remaining = msgBuffer->size();

		LoggingLoop(1, remaining);

		MirielEngine::Utils::GlobalLogger->log("Within Thread: Closing Logging File.");
		file.close();
	}

	std::string createLoggingFileName() {
		std::ostringstream os;
		std::time_t currTime = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		tm convertedCurrTime;
		localtime_s(&convertedCurrTime, &currTime);
		os << std::filesystem::current_path().string() << "/Logs/log - " << std::put_time(&convertedCurrTime, "%b-%d-%Y - %H-%M-%S") << ".log";
		return os.str();
	}

	Logger::Logger() {
		programRunning = true;
		msgQueue = std::make_shared<DataStructures::ThreadsafeQueue<std::unique_ptr<LoggingMessage>>>();

		loggingThread = std::thread(LoggingThreadFunction, msgQueue, currentLog);
	}

	Logger::~Logger() = default;

	void Logger::log(const std::string& msg) {
		auto lgMsg = std::make_unique<LoggingMessage>();
		lgMsg->msg = msg;
		lgMsg->time = std::chrono::system_clock::now();

		msgQueue->push(std::move(lgMsg));
	}

	Logger* Logger::getInstance() {
		if (instance == nullptr) {
			std::scoped_lock<std::mutex> lock(mtx);
			if (instance == nullptr) {
				instance = new Logger();
			}
		}
		return instance;
	}

	std::string Logger::getCurrentLog() {
		return currentLog;
	}

	bool Logger::getProgramRunning() {
		return programRunning;
	}

	void Logger::cleanup() {
		programRunning = false;
		loggingThread.join();
	}
}