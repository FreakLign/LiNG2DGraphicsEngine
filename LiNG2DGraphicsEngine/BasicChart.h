#pragma once
#include <glad/glad.h>
#ifdef _WIN64
#pragma message("--- [LiNG2DGraphics Link BEGIN]---")
#pragma message("    Compiled as Win64.")
#pragma message("--- [LiNG2DGraphics Link End]---")
#include <GLFW_x64/glfw3.h>
#ifdef _DLL
#pragma comment(lib, "x64/glfw3.lib")
#else
#pragma comment(lib, "x64/glfw3_mt.lib")
#endif
#pragma comment(lib, "x64/glew32.lib")
#else
#pragma message("--- [LiNG2DGraphics Link BEGIN]---")
#pragma message("    Compiled as Win32.")
#pragma message("--- [LiNG2DGraphics Link End]---")
#include <GLFW_x86/glfw3.h>
#ifdef _DLL
#pragma comment(lib, "x86/glfw3.lib")
#else
#pragma comment(lib, "x86/glfw3_mt.lib")
#endif
#pragma comment(lib, "x86/glew.32lib")
#endif
#include <gl/GL.h>
#pragma comment(lib, "opengl32.lib")

#include <iostream>
#include <thread>
#include <future>
#include <shared_mutex>

using namespace std;

class BasicChart
{
private:
	GLFWwindow* m_window = 0;
	thread m_renderThrd;
	GLfloat* m_bufferingData = 0;
	GLfloat* m_drawingData = 0;
	int m_bufferPos = 0;
	GLuint m_VAO = 0, m_VBO = 0;
	GLuint m_shaderProgram = 0;
	shared_timed_mutex m_mtx;

	int x_size = 2;
	const char* vertexShaderSource = "#version 440 core\n"
		"layout (location = 0) in vec3 aPos;\n"
		"void main()\n"
		"{\n"
		"   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
		"}\0";
	const char* fragmentShaderSource = "#version 440 core\n"
		"out vec4 FragColor;\n"
		"void main()\n"
		"{\n"
		"   FragColor = vec4(0.7f, 0.7f, 0.02f, 1.0f);\n"
		"}\n\0";
private:
	void Initialize() {
		glfwInit();

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	}

	void SetShader() {
		GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
		glCompileShader(vertexShader);
		GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
		glCompileShader(fragmentShader);
		InitProgram(vertexShader, fragmentShader);
	}

	void InitProgram(GLuint vertexShader, GLuint fragmentShader) {
		m_shaderProgram = glCreateProgram();
		glAttachShader(m_shaderProgram, vertexShader);
		glAttachShader(m_shaderProgram, fragmentShader);
		glLinkProgram(m_shaderProgram);
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
	}

	void BindBuffer() {
		glGenVertexArrays(1, &m_VAO);
		glGenBuffers(1, &m_VBO);
		glBindVertexArray(m_VAO);
		glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0);
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

public:
	BasicChart() {
		Initialize();
	}

	BasicChart(const BasicChart& chart) :BasicChart() {

	}

	BasicChart& operator=(const BasicChart& chart) {
		return *this;
	}

	BasicChart(int width, int height, const char* name) :BasicChart() {
		m_window = glfwCreateWindow(width, height, name, NULL, NULL);
		if (m_window == NULL)
		{
			std::cout << "Failed to create GLFW window" << std::endl;
			glfwTerminate();
			return;
		}
		glfwMakeContextCurrent(m_window);
		gladLoadGL();
	}
public:
	void SetVisualParas(int xpointcount) {
		x_size = xpointcount;
		m_bufferingData = new GLfloat[4 * size_t(x_size)];
		memset(m_bufferingData, 0, size_t(x_size) * 4ull * sizeof(GLfloat)); // Double Buffer
		m_drawingData = m_bufferingData;
	}

	void Start() {
		SetShader();
		BindBuffer();
		clock_t t0 = clock();
		int frameCounter = 0;
		while (!glfwWindowShouldClose(m_window)) {
			auto t1 = clock();
			if (t1 - t0 >= 500) {
				t0 = t1;
				cout << "FPS: " << frameCounter * 2 << endl;
				frameCounter = 0;
			}
			frameCounter++;
			glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT);
			glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
			//unique_lock<shared_timed_mutex>(m_mtx);
			glBufferData(GL_ARRAY_BUFFER, (size_t)x_size * 2 * sizeof(GLfloat), m_drawingData, GL_STATIC_DRAW);
			glUseProgram(m_shaderProgram);
			glBindVertexArray(m_VAO);
			glDrawArrays(GL_LINE_STRIP, 0, x_size);
			glfwSwapBuffers(m_window);
			glfwPollEvents();
		}
	}

	void InputData(float* datas, int dataCount) {
		int lastDataLen = dataCount;
		//unique_lock<shared_timed_mutex>(m_mtx);
		while (m_bufferPos + lastDataLen > x_size) {
			memcpy(m_bufferingData + m_bufferPos * 2, datas + size_t(dataCount - lastDataLen) * 2, (x_size - m_bufferPos) * 2 * sizeof(float));
			lastDataLen = lastDataLen - x_size + m_bufferPos;
			m_bufferPos = 0;
		}
		memcpy(m_bufferingData + m_bufferPos * 2, datas + (dataCount - lastDataLen) * 2, lastDataLen * 2 * sizeof(float));
		m_drawingData = m_bufferingData + m_bufferPos;
	}

	void Stop() {
		glDeleteVertexArrays(1, &m_VAO);
		glDeleteBuffers(1, &m_VBO);
		glDeleteProgram(m_shaderProgram);
		glfwDestroyWindow(m_window);
		glfwTerminate();
	}

	~BasicChart() {
		Stop();
	}
};

