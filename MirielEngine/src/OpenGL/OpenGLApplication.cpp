#include "OpenGL/OpenGLApplication.hpp"

#include <filesystem>

#include "DearImGui/imgui.h"
#include "DearImGui/imgui_impl_glfw.h"
#include "DearImGui/imgui_impl_opengl3.h"

namespace MirielEngine::OpenGL {

	OpenGLApplication::OpenGLApplication() {
		glfwInit();

		MirielEngine::Utils::GlobalLogger->log("OpenGL Application Launched.");

		window = NULL;

		try {
			window = MirielEngine::Core::createWindow(RENDER_BACKEND::OPENGL);
		} catch (MirielEngine::Errors::WindowCreationError& e) {
			glfwTerminate();
			throw MirielEngine::Errors::OpenGLError(e.what());
		}

		MirielEngine::Utils::GlobalLogger->log("Window Successfully Created.");
		MirielEngine::Utils::GlobalLogger->log("Making Context Current.");
		glfwMakeContextCurrent(window);

		MirielEngine::Utils::GlobalLogger->log("Attempting to Load Using GLAD.");
		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
			throw MirielEngine::Errors::OpenGLError("Failed to Load Using GLAD.");
		}

		MirielEngine::Utils::GlobalLogger->log("Adding Window Callbacks.");
		MirielEngine::Core::setWindowCallbacks(window);
		MirielEngine::Utils::GlobalLogger->log("Setting Up OpenGL Core.");

		MirielEngine::Utils::GlobalLogger->log("Setting Up ImGui.");

		// TODO: load in a settings file that contains font name, size, DPI scale
		fontName = std::filesystem::current_path().string() + "/src/Assets/Fonts/ProggyClean.ttf";
		fontSize = 12;
		highDPIScaleFactor = 2.0;

		try {
			initializeDearImGUI();
		} catch (MirielEngine::Errors::DearImGUIError& e) {
			glfwTerminate();
			ImGui::DestroyContext();
			throw MirielEngine::Errors::OpenGLError(e.what());
		}

		try {
			core = std::make_unique<MirielEngine::OpenGL::OpenGLCore>();
		} catch (MirielEngine::Errors::OpenGLError& e) {
			throw e;
		}

		gui = std::make_unique<MirielEngine::Utils::GUI>(core->getScene());
		// TODO
		/*make a callback class that will take the window and scene and then make all callback functions*/

		glfwGetWindowSize(window, &width, &height);
	}

	void OpenGLApplication::runApplication() {
		MirielEngine::Utils::GlobalLogger->log("Application has Started Main Loop.");
		/*
		* Each Function within this block will pretty much be within other files
		* Create Window + Callbacks will be within Utils
		* MirielEngine::Core::CreateWindow
		* MirielEngine::Core::SetCallbacks <- These will be within a Callback File
		*
		*/
		while (!glfwWindowShouldClose(window)) {
			glfwPollEvents();
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			core->draw(width, height);

			gui->generateFrame(ImGui_ImplOpenGL3_NewFrame);
			ImGui::Render();
			ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

			glfwSwapBuffers(window);
		}

		MirielEngine::Utils::GlobalLogger->log("Application has Exited Main Loop.");
	}

	void OpenGLApplication::cleanup() {
		MirielEngine::Utils::GlobalLogger->log("Shutting Down Backends for ImGui.");
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		MirielEngine::Utils::GlobalLogger->log("Destroying ImGui Context.");
		ImGui::DestroyContext();
		MirielEngine::Utils::GlobalLogger->log("Closing Window.");
		MirielEngine::Utils::GlobalLogger->log("Destroying Window.");
		glfwDestroyWindow(window);
		MirielEngine::Utils::GlobalLogger->log("Terminating GLFW.");
		glfwTerminate();

		/* Have to forcefully reset these to ensure that the scene share_ptr is released properly */
		gui.reset();
		core.reset();
	}

	void OpenGLApplication::initializeDearImGUI() {
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.Fonts->AddFontFromFileTTF(
			fontName.c_str(),
			fontSize * highDPIScaleFactor,
			NULL,
			NULL
		);

		ImGui::StyleColorsDark();

		if (!ImGui_ImplGlfw_InitForOpenGL(window, true)) { throw MirielEngine::Errors::DearImGUIError("ImGUI Failed to initialize GLFW for OpenGL"); }
		if (!ImGui_ImplOpenGL3_Init("#version 460 core")) { throw MirielEngine::Errors::DearImGUIError("ImGUI Failed to initialize OpenGL"); }
	}
}