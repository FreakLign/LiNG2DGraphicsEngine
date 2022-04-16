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
#include <ipps.h>

using namespace std;

class BasicChart
{
private:
	// GL Window handle.
	GLFWwindow* m_window = 0;

	// Data Buffering Memory.
	// The size of the buffer space is twice of point count.
	GLfloat* m_bufferingData = 0;

	// Index Buffer.
	// Initialized when the xpoint size was specified.
	GLfloat* m_indexBuffer = 0;

	// Drawing points data buffer. <Maybe not used.>
	GLfloat* m_drawingData = 0;

	GLfloat* m_drawingValues = 0;

	// The tail of buffered data.
	int m_bufferPos = 0;

	// GL Buffer.
	GLuint m_VAO = 0, m_VBO = 0;

	// Shader program.
	GLuint m_shaderProgram = 0;

	// Mutex lock.
	shared_timed_mutex m_mtx;

	// The count of 2d array datas.
	int x_size = 2;

	// The maxinum and minimum of the Y coordinat zone.
	GLdouble y_max = 0.f, y_min = 0.f;

	// Vertex shader source.
	const char* vertexShaderSource = "#version 440 core\n"
		"layout (location = 0) in vec3 aPos;\n"
		"void main()\n"
		"{\n"
		"   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
		"}\0";

	// Fragment shader source.
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

	void ComputePoints() {
		// Get rannge.
		GLdouble y_arrenge = abs(y_max - y_min);
		GLdouble y_mid = (y_max + y_min) / 2.0;

		// Convert the values to y values of points.
		ippsSubC_32f(m_bufferingData + m_bufferPos, GLfloat(y_mid), m_drawingValues, x_size);
		ippsDivC_32f_I(GLfloat(y_arrenge / 2.0), m_bufferingData + m_bufferPos, x_size);

		// Combine index and values to points.
		ippsRealToCplx_32f(m_indexBuffer, m_drawingValues, (Ipp32fc*)m_drawingData, x_size);
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
		gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		gladLoadGL();
	}
public:
	void SetVisualParas(int xpointcount, double ymin, double ymax) {
		x_size = xpointcount;
		y_max = ymax;
		y_min = ymin;

		// Reset the tail position to 0.
		m_bufferPos = 0;

		// Reconstruct the drawing values buffer.
		if (m_drawingValues != 0) delete[] m_drawingValues;
		m_drawingValues = new GLfloat[x_size];

		// Reconstruct the data buffer.
		if (m_bufferingData != 0) delete[] m_bufferingData;
		m_bufferingData = new GLfloat[2 * size_t(x_size)];
		memset(m_bufferingData, 0, 2 * size_t(x_size) * sizeof(GLfloat));

		// Reconstruct the index buffer.
		if (m_indexBuffer != 0) delete[] m_indexBuffer;
		m_indexBuffer = new GLfloat[x_size];

		// Initialize the index values.
		for (size_t i = 0; i < x_size; i++)
		{
			m_indexBuffer[i] = (float(i) - float(float(x_size) / 2.f)) / (float(x_size) / 2.f);
		}

		// Initailize the drawing points buffer.
		m_drawingData = new GLfloat[2 * size_t(x_size)];

		ComputePoints();
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
			//m_mtx.lock();
			glBufferData(GL_ARRAY_BUFFER, (size_t)x_size * 2 * sizeof(GLfloat), m_drawingData, GL_DYNAMIC_DRAW);
			glUseProgram(m_shaderProgram);
			glBindVertexArray(m_VAO);
			glDrawArrays(GL_LINE_STRIP, 0, x_size);
			glfwSwapBuffers(m_window);
			//m_mtx.unlock();
			glfwPollEvents();
		}
	}

	void InputData(float* datas, int dataCount) {
		auto tempData = datas;
		// Remove additional datas.
		// These part of data is not going to render in this frame.
		if (dataCount + m_bufferPos >= x_size) {
			tempData = datas + m_bufferPos + dataCount - x_size;
			memcpy(m_bufferingData, m_bufferingData + x_size, x_size * sizeof(float));
			m_bufferPos -= x_size;
		}
		size_t posShift = size_t(x_size + m_bufferPos);
		memcpy(m_bufferingData + posShift, tempData, (dataCount) * sizeof(float));
		m_bufferPos += dataCount;
		ComputePoints();
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

