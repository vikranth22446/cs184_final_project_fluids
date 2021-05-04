#include "main.h"
#include "shader.h"
#include "particle.h"
#include "particlesim.h"
#include "PointCloudAdapter.h"

ParticleSim *app = nullptr;
GLFWwindow *window = nullptr;
Screen *screen = nullptr;

void error_callback(int error, const char *description)
{
	puts(description);
}

// copied from opengl proj4
void createGLContexts()
{
	if (!glfwInit())
	{
		return;
	}

	glfwSetTime(0);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	glfwWindowHint(GLFW_SAMPLES, 0);
	glfwWindowHint(GLFW_RED_BITS, 8);
	glfwWindowHint(GLFW_GREEN_BITS, 8);
	glfwWindowHint(GLFW_BLUE_BITS, 8);
	glfwWindowHint(GLFW_ALPHA_BITS, 8);
	glfwWindowHint(GLFW_STENCIL_BITS, 8);
	glfwWindowHint(GLFW_DEPTH_BITS, 24);
	glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

	// Create a GLFWwindow object
	window = glfwCreateWindow(800, 800, "Fluid Simulation", nullptr, nullptr);
	if (window == nullptr)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return;
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		throw std::runtime_error("Could not initialize GLAD!");
	}
	glGetError(); // pull and ignore unhandled errors like GL_INVALID_ENUM

	glClearColor(0.2f, 0.25f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	// Create a nanogui screen and pass the glfw pointer to initialize
	screen = new Screen();
	screen->initialize(window, true);

	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);
	glfwSwapInterval(1);
	glfwSwapBuffers(window);
}

void setGLFWCallbacks()
{
	glfwSetCursorPosCallback(window, [](GLFWwindow *, double x, double y) {
		if (!screen->cursorPosCallbackEvent(x, y))
		{
			app->cursorPosCallbackEvent(x / screen->pixelRatio(),
										y / screen->pixelRatio());
		}
	});

	glfwSetMouseButtonCallback(
		window, [](GLFWwindow *, int button, int action, int modifiers) {
			if (!screen->mouseButtonCallbackEvent(button, action, modifiers) ||
				action == GLFW_RELEASE)
			{
				app->mouseButtonCallbackEvent(button, action, modifiers);
			}
		});

	glfwSetKeyCallback(
		window, [](GLFWwindow *, int key, int scancode, int action, int mods) {
			if (!screen->keyCallbackEvent(key, scancode, action, mods))
			{
				app->keyCallbackEvent(key, scancode, action, mods);
			}
		});

	glfwSetCharCallback(window, [](GLFWwindow *, unsigned int codepoint) {
		screen->charCallbackEvent(codepoint);
	});

	glfwSetDropCallback(window,
						[](GLFWwindow *, int count, const char **filenames) {
							screen->dropCallbackEvent(count, filenames);
							app->dropCallbackEvent(count, filenames);
						});

	glfwSetScrollCallback(window, [](GLFWwindow *, double x, double y) {
		if (!screen->scrollCallbackEvent(x, y))
		{
			app->scrollCallbackEvent(x, y);
		}
	});

	glfwSetFramebufferSizeCallback(window,
								   [](GLFWwindow *, int width, int height) {
									   screen->resizeCallbackEvent(width, height);
									   app->resizeCallbackEvent(width, height);
								   });
}

int main(void)
{
	testPointCloud();
	// Initialise GLFW
	glfwSetErrorCallback(error_callback);
	createGLContexts();
	// Call this after all the widgets have been defined
	app = new ParticleSim(screen, window, MAX_PARTICLES);

	screen->setVisible(true);
	screen->performLayout();
	setGLFWCallbacks();

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	glm::vec4 grayBackround = glm::vec4(.25f);
	glm::vec4 whiteBackground = glm::vec4(1.0f);
	glm::vec4 background = whiteBackground;

	// Dark blue background
	glClearColor(background.x, background.y, background.b, background.a);
	
	// fps counter logic https://www.opengl-tutorial.org/miscellaneous/an-fps-counter/
	int nbFrames = 0;
	double lastTime = glfwGetTime();
	double lastFPSTime = glfwGetTime();
	app->init();

	// The VBO containing the 4 vertices of the particles.
	// Thanks to instancing, they will be shared by all particles.
	do
	{
		glfwSwapBuffers(window);
		glfwPollEvents();

		glClearColor(background.x, background.y, background.b, background.a);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		app->drawContents();
		screen->drawContents();
		screen->drawWidgets();

		double currentTime = glfwGetTime();
		double delta = currentTime - lastTime;
		lastTime = currentTime;
		nbFrames++;
		if (currentTime - lastFPSTime >= 1.0)
		{ // If last prinf() was more than 1 sec ago
			// printf and reset timer
			printf("ms/frame: %f fps: %f\n", 1000.0 / double(nbFrames), double(nbFrames) / 1);
			nbFrames = 0;
			lastFPSTime += 1.0;
		}
		
		if (!app->isAlive()) {
			glfwSetWindowShouldClose(window, 1);
		}
	} // Check if the ESC key was pressed or the window was closed
	while (glfwWindowShouldClose(window) == 0);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();
	return 0;
}
