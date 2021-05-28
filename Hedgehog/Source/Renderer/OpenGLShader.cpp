#include <Renderer/OpenGLShader.h>

#include <fstream>
#include <glm/gtc/type_ptr.hpp>


namespace Hedge
{

OpenGLShader::OpenGLShader(const std::string& vertexFilePath,
						   const std::string& pixelFilePath,
						   const std::string& geometryFilePath)
{
	bool useGeometryShader = !geometryFilePath.empty();

	GLuint vertexShader = CompileShader(GL_VERTEX_SHADER, vertexFilePath);
	if (vertexShader == 0)
	{
		return;
	}

	GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, pixelFilePath);
	if (fragmentShader == 0)
	{
		// Clean-up the previous shaders
		glDeleteShader(vertexShader);
		return;
	}

	GLuint geometryShader = 0;
	if (useGeometryShader)
	{
		geometryShader = CompileShader(GL_GEOMETRY_SHADER, geometryFilePath);
		if (geometryShader == 0)
		{
			// Clean-up the previous shaders
			glDeleteShader(vertexShader);
			glDeleteShader(fragmentShader);
			return;
		}
	}

	// All shaders are successfully compiled.
	// Now time to link them together into a program.
	// Get a program object.
	shaderID = glCreateProgram();

	// Attach our shaders to our program
	glAttachShader(shaderID, vertexShader);
	glAttachShader(shaderID, fragmentShader);
	if (useGeometryShader)
	{
		glAttachShader(shaderID, geometryShader);
	}

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

		// Clean-up the program
		glDeleteProgram(shaderID);
		// Clean-up the shaders
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
		if (useGeometryShader)
		{
			glDeleteShader(geometryShader);
		}

		// Use the infoLog as you see fit.
		printf("Program linking error.\n");
		printf("%s", infoLog.data());

		// In this simple program, we'll just leave
		return;
	}

	// Always detach and delete shaders after a successful link.
	glDetachShader(shaderID, vertexShader);
	glDeleteShader(vertexShader);
	glDetachShader(shaderID, fragmentShader);
	glDeleteShader(fragmentShader);
	if (useGeometryShader)
	{
		glDetachShader(shaderID, geometryShader);
		glDeleteShader(geometryShader);
	}
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

void OpenGLShader::UploadConstant(const std::string& name, const std::vector<glm::mat4>& constant)
{
	if (constant.size() == 1)
	{
		UploadConstant(name, constant[0]);
	}
	else
	{
		for (int i = 0; i < constant.size(); i++)
		{
			UploadConstant(name + "[" + std::to_string(i) + "]", constant[i]);
		}
	}
}

void OpenGLShader::UploadConstant(const std::string& name, int constant)
{
	Bind();
	GLint uniformLocation = glGetUniformLocation(shaderID, name.c_str());
	glUniform1i(uniformLocation, constant);
}

void OpenGLShader::UploadConstant(const std::string& name, const DirectionalLight& constant)
{
	UploadConstant(name + ".color", constant.color);
	UploadConstant(name + ".direction", constant.direction);
}

void OpenGLShader::UploadConstant(const std::string& name, const PointLight& constant)
{
	UploadConstant(name + ".color", constant.color);
	UploadConstant(name + ".position", constant.position);
	UploadConstant(name + ".attenuation", constant.attenuation);
}

void OpenGLShader::UploadConstant(const std::string& name, const SpotLight& constant)
{
	UploadConstant(name + ".color", constant.color);
	UploadConstant(name + ".position", constant.position);
	UploadConstant(name + ".attenuation", constant.attenuation);
	UploadConstant(name + ".direction", constant.direction);
	UploadConstant(name + ".cutoffAngle", constant.cutoffAngle);
}

void OpenGLShader::UploadConstant(const std::string& name, const DirectionalLight* constant, int count)
{
	if (count == 1)
	{
		UploadConstant(name, *constant);
	}
	else
	{
		for (int i = 0; i < count; i++)
		{
			UploadConstant(name + "[" + std::to_string(i) + "].color", constant[i].color);
			UploadConstant(name + "[" + std::to_string(i) + "].direction", constant[i].direction);
		}
	}
}

void OpenGLShader::UploadConstant(const std::string& name, const PointLight* constant, int count)
{
	if (count == 1)
	{
		UploadConstant(name, *constant);
	}
	else
	{
		for (int i = 0; i < count; i++)
		{
			UploadConstant(name + "[" + std::to_string(i) + "].color", constant[i].color);
			UploadConstant(name + "[" + std::to_string(i) + "].position", constant[i].position);
			UploadConstant(name + "[" + std::to_string(i) + "].attenuation", constant[i].attenuation);
		}
	}
}

void OpenGLShader::UploadConstant(const std::string& name, const SpotLight* constant, int count)
{
	if (count == 1)
	{
		UploadConstant(name, *constant);
	}
	else
	{
		for (int i = 0; i < count; i++)
		{
			UploadConstant(name + "[" + std::to_string(i) + "].color", constant[i].color);
			UploadConstant(name + "[" + std::to_string(i) + "].position", constant[i].position);
			UploadConstant(name + "[" + std::to_string(i) + "].attenuation", constant[i].attenuation);
			UploadConstant(name + "[" + std::to_string(i) + "].direction", constant[i].direction);
			UploadConstant(name + "[" + std::to_string(i) + "].cutoffAngle", constant[i].cutoffAngle);
		}
	}
}

void OpenGLShader::UploadConstant(const std::string& name, const void* constant, unsigned long long size)
{
	assert(false);
}

GLuint OpenGLShader::CompileShader(GLenum shaderType, const std::string& srcFilePath)
{
	// Create an empty shader handle
	GLuint shader = glCreateShader(shaderType);

	// Send the shader source code to GL
	// Note that std::string's .c_str is NULL character terminated.
	std::string shaderSrc = ReadFile(srcFilePath);
	const GLchar* source = (const GLchar*)shaderSrc.c_str();
	glShaderSource(shader, 1, &source, 0);

	// Compile the shader
	glCompileShader(shader);

	// Check the compilation status
	GLint isCompiled = 0;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
	if (isCompiled == GL_FALSE)
	{
		std::vector<GLchar> infoLog = getShaderInfoLog(shader);

		// We don't need the shader anymore.
		glDeleteShader(shader);

		// Use the infoLog as you see fit.
		printf("'%s' compilation error.\n", srcFilePath.c_str());
		printf("%s", infoLog.data());

		// In this simple program, we'll just leave
		return 0;
	}

	return shader;
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

	return infoLog;
}

} // namespace Hedge
