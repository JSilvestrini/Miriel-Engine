#include "Utils/MirielEngineCore.hpp"

#include <string>
#include <sstream>

namespace MirielEngine::Core {

	void StartBackend(RENDER_BACKEND backend) {
		MirielEngine::Utils::GlobalLogger->log("Currently Choosing Backend within Miriel Engine Core.");
		switch (backend) {
			using enum RENDER_BACKEND;
			case DX12:
				#if _WIN64
				// TODO
				// MirielEngine::DX12::RunApplication
				#endif
				break;
			case OPENGL:
				#if _WIN64
				try {
					auto openGLApplication = std::make_unique<MirielEngine::OpenGL::OpenGLApplication>();
					openGLApplication->runApplication();
					openGLApplication->cleanup();
					MirielEngine::Utils::GlobalLogger->log("OpenGL Application Finished Running.");
				} catch (MirielEngine::Errors::OpenGLError& e) {
					MirielEngine::Utils::GlobalLogger->log(e.what());
				} catch (std::runtime_error& e) {
					MirielEngine::Utils::GlobalLogger->log("Uncaught Error:");
					MirielEngine::Utils::GlobalLogger->log(e.what());
				}
				#endif
				break;
			case VULKAN:
				#if _WIN64
				// TODO
				// MirielEngine::Vulkan::RunApplication
				#endif
				break;
			case METAL:
				#if __APPLE__
				// TODO
				// MirielEngine::Metal::RunApplication
				#endif
				break;
			default:
				// Technically impossible to get here
				std::ostringstream out;
				out << "Incorrect Backend Enumation Given: " << int(backend) << " Within " << __FILE__ << " at Line: " << __LINE__;
				MirielEngine::Utils::GlobalLogger->log(out.str());
				throw MirielEngine::Errors::CoreError(out.str().c_str());
		}
		MirielEngine::Utils::GlobalLogger->log("Core Application Finished Running.");
	}
}