#include <Renderer/OpenGLContext.h>

#include <cstdio>

// GLAD completely replaces GL.h
// If there is something missing it means that the glad files were not generated properly,
// for example glBegin, glColor3f, glEnd are in the compatibility profile and not in the core
#include <glad/glad.h>


// GLAD_WGL completely replaces any WGL extensions you would load using wglext.h and something like example for swap_interval below
//bool WGLExtensionSupported(const char* extension_name)
//{
//    // this is pointer to function which returns pointer to string with list of all wgl extensions
//    PFNWGLGETEXTENSIONSSTRINGEXTPROC _wglGetExtensionsStringEXT = NULL;
//
//    // determine pointer to wglGetExtensionsStringEXT function
//    _wglGetExtensionsStringEXT = (PFNWGLGETEXTENSIONSSTRINGEXTPROC)wglGetProcAddress("wglGetExtensionsStringEXT");
//
//    if (strstr(_wglGetExtensionsStringEXT(), extension_name) == NULL)
//    {
//        // string was not found
//        return false;
//    }
//
//    // extension is supported
//    return true;
//}

//if (WGLExtensionSupported("WGL_EXT_swap_control"))
//{
//    printf("WGL_EXT_swap_control exists\n");
//    // Extension is supported, init pointers.
//    PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");

//    // this is another function from WGL_EXT_swap_control extension
//    PFNWGLGETSWAPINTERVALEXTPROC wglGetSwapIntervalEXT = (PFNWGLGETSWAPINTERVALEXTPROC)wglGetProcAddress("wglGetSwapIntervalEXT");

//    wglSwapIntervalEXT(1);
//}
//
// If there is something missing it means that the glad files were not generated properly,
// for example, WGL_EXT_swap_control has WGL_EXT_extensions_string as a dependency
#include <glad/glad_wgl.h>


namespace Hedge
{

OpenGLContext::OpenGLContext(HWND windowHandle)
{
	this->windowHandle = windowHandle;

	deviceContext = GetDC(windowHandle);

	// Create OpenGL context
	PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,    // Flags
		PFD_TYPE_RGBA,        // The kind of framebuffer. RGBA or palette.
		32,                   // Colordepth of the framebuffer.
		0, 0, 0, 0, 0, 0,
		0,
		0,
		0,
		0, 0, 0, 0,
		24,                   // Number of bits for the depthbuffer
		8,                    // Number of bits for the stencilbuffer
		0,                    // Number of Aux buffers in the framebuffer.
		PFD_MAIN_PLANE,
		0,
		0, 0, 0
	};

	int pf = ChoosePixelFormat(deviceContext, &pfd);
	SetPixelFormat(deviceContext, pf, &pfd);

	context = wglCreateContext(deviceContext);
	wglMakeCurrent(deviceContext, context);


	// Load GL using GLAD's default loader
	if (!gladLoadGL())
	{
		printf("Error gladLoadGL()\n");
	}

	// Load WGL using GLAD's default loader
	if (!gladLoadWGL(deviceContext))
	{
		printf("Error gladLoadWGL()\n");
	}

	glEnable(GL_SCISSOR_TEST);
}

OpenGLContext::~OpenGLContext()
{
	wglDeleteContext(context);
	DeleteDC(deviceContext);
}

void OpenGLContext::SetSwapInterval(int interval)
{
	wglSwapIntervalEXT(interval);
}

void OpenGLContext::SwapBuffers()
{
	//wglMakeCurrent(deviceContext, context);
	::SwapBuffers(deviceContext);
}

void OpenGLContext::MakeCurrent()
{
	wglMakeCurrent(deviceContext, context);
}

} // namespace Hedge
