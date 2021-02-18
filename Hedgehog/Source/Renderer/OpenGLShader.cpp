#include <Renderer/OpenGLShader.h>

#include <fstream>
#include <glm/gtc/type_ptr.hpp>


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
		std::vector<GLchar> infoLog = getShaderInfoLog(fragmentShader);

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
	shaderID = glCreateProgram();

	// Attach our shaders to our program
	glAttachShader(shaderID, vertexShader);
	glAttachShader(shaderID, fragmentShader);

	// Link our program
	glLinkProgram(shaderID);

	// Note the different functions here: glGetProgram* instead of glGetShader*.
	GLint isLinked = 0;
	glGetProgramiv(shaderID, GL_LINK_STATUS, (int*)&isLinked);
	if (isLinked == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetProgramiv(shaderID, GL_INFO_LOG_LENGTH, &maxLength);

		// The maxLength includes the NULL character
		std::vector<GLchar> infoLog(maxLength);
		glGetProgramInfoLog(shaderID, maxLength, &maxLength, &infoLog[0]);

		// We don't need the program anymore.
		glDeleteProgram(shaderID);
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
	glDetachShader(shaderID, vertexShader);
	glDetachShader(shaderID, fragmentShader);
}

OpenGLShader::~OpenGLShader()
{
	glUseProgram(0);
	glDeleteProgram(shaderID);
}

void OpenGLShader::Bind()
{
	glUseProgram(shaderID);
}

void OpenGLShader::Unbind()
{
	glUseProgram(0);
}

void OpenGLShader::UploadConstant(const std::string& name, float constant)
{
	Bind();
	GLint uniformLocation = glGetUniformLocation(shaderID, name.c_str());
	glUniform1f(uniformLocation, constant);
}

void OpenGLShader::UploadConstant(const std::string& name, glm::vec2 constant)
{
	Bind();
	GLint uniformLocation = glGetUniformLocation(shaderID, name.c_str());
	glUniform2f(uniformLocation, constant[0], constant[1]);
}

void OpenGLShader::UploadConstant(const std::string& name, glm::vec3 constant)
{
	Bind();
	GLint uniformLocation = glGetUniformLocation(shaderID, name.c_str());
	glUniform3f(uniformLocation, constant[0], constant[1], constant[2]);
}

void OpenGLShader::UploadConstant(const std::string& name, glm::vec4 constant)
{
	Bind();
	GLint uniformLocation = glGetUniformLocation(shaderID, name.c_str());
	glUniform4f(uniformLocation, constant[0], constant[1], constant[2], constant[3]);
}

void OpenGLShader::UploadConstant(const std::string& name, glm::mat3x3 constant)
{
	Bind();
	GLint uniformLocation = glGetUniformLocation(shaderID, name.c_str());
	glUniformMatrix3fv(uniformLocation, 1, GL_FALSE, glm::value_ptr(constant));
}

void OpenGLShader::UploadConstant(const std::string& name, glm::mat4x4 constant)
{
	Bind();
	GLint uniformLocation = glGetUniformLocation(shaderID, name.c_str());
	// the program must be bound first to the context with glUseProgram
	// value_ptr() returns a direct pointer to the matrix data in column-major order, making it useful for uploading data to OpenGL
	glUniformMatrix4fv(uniformLocation, 1, GL_FALSE, glm::value_ptr(constant));

	// If using OpenGL 4.1 or ARB_separate_shader_objects, you may use the glProgramUniform* functions to set uniforms directly on a program,
	// without having to bind the program first
	//glProgramUniformMatrix4fv(shaderID, uniformLocation, 1, GL_FALSE, glm::value_ptr(constant));
}

void OpenGLShader::UploadConstant(const std::string& name, int constant)
{
	Bind();
	GLint uniformLocation = glGetUniformLocation(shaderID, name.c_str());
	glUniform1i(uniformLocation, constant);
}

void OpenGLShader::UploadConstant(const std::string& name, void* constant, unsigned long long size)
{
	assert(false);
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
