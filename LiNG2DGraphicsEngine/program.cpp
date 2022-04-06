#include "BasicChart.h"

#include <iostream>

using namespace std;

void Initialize() {
	glfwInit();
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
}

GLFWwindow* CreateGLWindow(int width, int height, const char* windowName) {
	GLFWwindow* window;
	window = glfwCreateWindow(width, height, windowName, 0, 0);
	if (window == 0) {
		cout << "Create Window Error" << endl;
		return 0;
	}
	glfwMakeContextCurrent(window);
	if (glewInit() != GLEW_OK) {
		cout << "Initial Window Error" << endl;
		return 0;
	}
	return window;
}

int main() {
	Initialize();
	GLFWwindow* window = CreateGLWindow(640, 480, "Test");
	if (window == 0)return -1;
	while (1) {
		glClear(GL_COLOR_BUFFER_BIT);
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glfwDestroyWindow(window);
	glfwTerminate();
}