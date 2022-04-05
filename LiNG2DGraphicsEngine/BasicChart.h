#pragma once
#include <GL/glew.h>
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


class BasicChart
{
public:
	void Initialize(void* handle);
};

