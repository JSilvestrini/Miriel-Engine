#include "CustomErrors/MirielEngineErrors.hpp"

namespace MirielEngine::Errors {
	/*				CORE ERROR				*/
	CoreError::CoreError(char const* const message) : message(message) {}

	char const* CoreError::what() const {
		return message;
	}

	/*				LOGGING ERROR				*/
	LoggingError::LoggingError(char const* const message) : message(message) {}

	char const* LoggingError::what() const {
		return message;
	}

	/*			MAIN FUNCTION ERROR			*/
	MainFunctionError::MainFunctionError(char const* const message) : message(message) {}

	char const* MainFunctionError::what() const {
		return message;
	}

	/*			WINDOW CREATION ERROR			*/
	WindowCreationError::WindowCreationError(char const* const message) : message(message) {}

	char const* WindowCreationError::what() const {
		return message;
	}

	/*				OPENGL ERROR				*/
	OpenGLError::OpenGLError(char const* const message) : message(message) {}

	char const* OpenGLError::what() const {
		return message;
	}

	/*			OPENGL UTILS ERROR				*/
	OpenGLUtilError::OpenGLUtilError(char const* const message) : message(message) {}

	char const* OpenGLUtilError::what() const {
		return message;
	}

	/*			OBJECT LOADER ERROR			*/
	ObjectLoaderError::ObjectLoaderError(char const* const message) : message(message) {}

	char const* ObjectLoaderError::what() const {
		return message;
	}
}