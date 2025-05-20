#define STB_IMAGE_IMPLEMENTATION

#include <stb_image.h>
//#define STB_IMAGE_WRITE_IMPLEMENTATION <- might need this is I choose to perform image compression or use GPU to create noise textures
#include "Utils/mainUtils.hpp"
#include "Utils/MirielEngineCore.hpp"
#include "Utils/MirielEngineLogger.hpp"

MirielEngine::Utils::Logger* MirielEngine::Utils::Logger::instance = nullptr;
std::mutex MirielEngine::Utils::Logger::mtx;

int main(int argc, char* argv[]) {
	checkLoggingDir();
	MirielEngine::Utils::GlobalLogger->log("Checking main.cpp Arguments.");

	if (argc < 2) {
		MirielEngine::Utils::GlobalLogger->log("Not Enough Arguments: Expected at Least 2 Additional Arguments.");
		MirielEngine::Utils::GlobalLogger->cleanup();
		throw MirielEngine::Errors::MainFunctionError("Not Enough Arguments: Expected at Least 2 Additional Arguments.");
	}

	if (std::string(argv[1]) != std::string("-e")) {
		MirielEngine::Utils::GlobalLogger->log("Incorrect Arguments: argv[1] Should be '-e'.");
		MirielEngine::Utils::GlobalLogger->cleanup();
		throw MirielEngine::Errors::MainFunctionError("Incorrect Arguments: argv[1] Should be '-e'.");
	}

	RENDER_BACKEND backend;

	if (std::string(argv[2]) == std::string("DX12")) {
		backend = RENDER_BACKEND::DX12;
		MirielEngine::Utils::GlobalLogger->log("Using DirectX12 Backend.");
	} else if (std::string(argv[2]) == std::string("OpenGL")) {
		backend = RENDER_BACKEND::OPENGL;
		MirielEngine::Utils::GlobalLogger->log("Using OpenGL Backend.");
	} else if (std::string(argv[2]) == std::string("Vulkan")) {
		backend = RENDER_BACKEND::VULKAN;
		MirielEngine::Utils::GlobalLogger->log("Using Vulkan Backend.");
	} else {
		MirielEngine::Utils::GlobalLogger->log("Expected One of the Following After '-e': 'DX12', 'OpenGL', or 'Vulkan'.");
		MirielEngine::Utils::GlobalLogger->cleanup();
		throw MirielEngine::Errors::MainFunctionError("Expected One of the Following After '-e': 'DX12', 'OpenGL', or 'Vulkan'.");
	}

	try {
		MirielEngine::Core::StartBackend(backend);
	} catch (MirielEngine::Errors::CoreError& e) {
		MirielEngine::Utils::GlobalLogger->log(e.what());
		MirielEngine::Utils::GlobalLogger->log("Program Finished Running: Cleaning Up Logger.");
		MirielEngine::Utils::GlobalLogger->cleanup();
	}

	MirielEngine::Utils::GlobalLogger->log("Program Finished Running: Cleaning Up Logger.");
	MirielEngine::Utils::GlobalLogger->cleanup();
}