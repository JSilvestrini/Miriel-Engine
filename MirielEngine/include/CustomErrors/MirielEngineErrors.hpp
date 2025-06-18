#pragma once
#include <stdexcept>

namespace MirielEngine::Errors {
	class CoreError : public std::exception {
		private:
			const char* message;
		public:
			CoreError(char const* const message);
			virtual char const* what() const;
	};

	class LoggingError : public std::exception {
		private:
			const char* message;
		public:
			LoggingError(char const* const message);
			virtual char const* what() const;
	};

	class MainFunctionError : public std::exception {
		private:
			const char* message;
		public:
			MainFunctionError(char const* const message);
			virtual char const* what() const;
	};

	class WindowCreationError : public std::exception {
		private:
			const char* message;
		public:
			WindowCreationError(char const* const message);
			virtual char const* what() const;
	};

	class OpenGLError : public std::exception {
		private:
			const char* message;
		public:
			OpenGLError(char const* const message);
			virtual char const* what() const;
	};

	class DearImGUIError : public std::exception {
		private:
		const char* message;
		public:
		DearImGUIError(char const* const message);
		virtual char const* what() const;
	};

	class OpenGLUtilError : public std::exception {
		private:
		const char* message;
		public:
		OpenGLUtilError(char const* const message);
		virtual char const* what() const;
	};

	class ObjectLoaderError : public std::exception {
		private:
			const char* message;
		public:
			ObjectLoaderError(char const* const message);
			virtual char const* what() const;
	};

	// TODO: Vulkan and DX12 Errors
}