#include <Renderer/OpenGLShader.h>

#include <fstream>


OpenGLShader::OpenGLShader(const std::string& vertexFilePath, const std::string& pixelFilePath)
{
	// Create an empty vertex shader handle
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);

	// Send the vertex shader source code to GL
	// Note that std::string's .c_str is NULL character terminated.
	std::string vertexSrc = ReadFile(vertexFilePath);
	const GLchar* source = (const GLchar*)vertexSrc.c_str();
	glShaderSource(vertexShader, 1, &source, 0);

	// Compile the vertex shader
	glCompileShader(vertexShader);

	GLint isCompiled = 0;
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &isCompiled);
	if (isCompiled == GL_FALSE)
	{
		std::vector<GLchar> infoLog = getShaderInfoLog(vertexShader);

		// Use the infoLog as you see fit.
		printf("Vertex shader compilation error.\n");
		printf("%s", infoLog.data());

		// In this simple program, we'll just leave
		return;
	}

	// Create an empty fragment shader handle
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	// Send the fragment shader source code to GL
	// Note that std::string's .c_str is NULL character terminated.
	std::string fragmentSrc = ReadFile(pixelFilePath);
	source = (const GLchar*)fragmentSrc.c_str();
	glShaderSource(fragmentShader, 1, &source, 0);

	// Compile the fragment shader
	glCompileShader(fragmentShader);

	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &isCompiled);
	if (isCompiled == GL_FALSE)
	{
		std::vector<GLchar> infoLog = getShaderInfoLog(vertexShader);

		// We don't need the shader anymore. Don't leak shaders.
		glDeleteShader(vertexShader);

		// Use the infoLog as you see fit.
		printf("Fragment shader compilation error.\n");
		printf("%s", infoLog.data());

		// In this simple program, we'll just leave
		return;
	}

	// Vertex and fragment shaders are successfully compiled.
	// Now time to link them together into a program.
	// Get a program object.
	rendererID = glCreateProgram();

	// Attach our shaders to our program
	glAttachShader(rendererID, vertexShader);
	glAttachShader(rendererID, fragmentShader);

	// Link our program
	glLinkProgram(rendererID);

	// Note the different functions here: glGetProgram* instead of glGetShader*.
	GLint isLinked = 0;
	glGetProgramiv(rendererID, GL_LINK_STATUS, (int*)&isLinked);
	if (isLinked == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetProgramiv(rendererID, GL_INFO_LOG_LENGTH, &maxLength);

		// The maxLength includes the NULL character
		std::vector<GLchar> infoLog(maxLength);
		glGetProgramInfoLog(rendererID, maxLength, &maxLength, &infoLog[0]);

		// We don't need the program anymore.
		glDeleteProgram(rendererID);
		// Don't leak shaders either.
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);

		// Use the infoLog as you see fit.
		printf("Program linking error.\n");
		printf("%s", infoLog.data());

		// In this simple program, we'll just leave
		return;
	}

	// Always detach shaders after a successful link.
	glDetachShader(rendererID, vertexShader);
	glDetachShader(rendererID, fragmentShader);
}

OpenGLShader::~OpenGLShader()
{
	glDeleteProgram(rendererID);
}

void OpenGLShader::Bind() const
{
	glUseProgram(rendererID);
}

void OpenGLShader::Unbind() const
{
	glUseProgram(0);
}

std::string OpenGLShader::ReadFile(const std::string& filePath)
{
	std::string content;
	std::ifstream in(filePath);

	if (in)
	{
		//content = std::string((std::istreambuf_iterator<char>(in)),
		//					  std::istreambuf_iterator<char>());

		in.seekg(0, std::ios::end);
		content.resize(in.tellg());
		in.seekg(0, std::ios::beg);
		in.read(&content[0], content.size());
		in.close();
	}
	else
	{
		printf("Couldn't open %s\n", filePath.c_str());
	}

	return content;
}

std::vector<GLchar> OpenGLShader::getShaderInfoLog(GLint id)
{
	GLint maxLength = 0;
	glGetShaderiv(id, GL_INFO_LOG_LENGTH, &maxLength);

	// The maxLength includes the NULL character
	std::vector<GLchar> infoLog(maxLength);
	glGetShaderInfoLog(id, maxLength, &maxLength, &infoLog[0]);

	// We don't need the shader anymore.
	glDeleteShader(id);

	return infoLog;
}
