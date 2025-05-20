#include "Utils/mainUtils.hpp"

#include <sstream>
#include <filesystem>

void checkLoggingDir() {
	MirielEngine::Utils::GlobalLogger->log("Attempting to Create Logging Directory.");
	if (std::filesystem::exists(std::filesystem::path(std::filesystem::current_path().string() + "/Logs"))) {
		MirielEngine::Utils::GlobalLogger->log("Logging Directory Exists.");
		MirielEngine::Utils::GlobalLogger->log("Resetting Permissions on Directory.");
		std::filesystem::permissions(std::filesystem::path(std::filesystem::current_path().string() + "/Logs"), std::filesystem::perms::all);
		return;
	}

	MirielEngine::Utils::GlobalLogger->log("Creating Logging Directory.");
	std::filesystem::create_directory(std::filesystem::path(std::filesystem::current_path().string() + "/Logs"));
	MirielEngine::Utils::GlobalLogger->log("Setting Permissions on Directory.");
	std::filesystem::permissions(std::filesystem::path(std::filesystem::current_path().string() + "/Logs"), std::filesystem::perms::all);
}