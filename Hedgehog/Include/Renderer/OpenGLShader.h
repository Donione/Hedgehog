#pragma once

#include <Renderer/Shader.h>

#include <glad/glad.h>
#include <vector>


namespace Hedge
{

class OpenGLShader : public Shader
{
public:
	OpenGLShader(const std::string& vertexFilePath,
				 const std::string& pixelFilePath,
				 const std::string& geometryFilePath = "");
	virtual ~OpenGLShader() override;

	virtual void Bind() override;
	virtual void Unbind() override;

	virtual void SetupConstantBuffers(ConstantBufferDescription constBufferDesc) override { /* do nothing */ }

	virtual void UploadConstant(const std::string& name, float constant) override;
	virtual void UploadConstant(const std::string& name, glm::vec2 constant) override;
	virtual void UploadConstant(const std::string& name, glm::vec3 constant) override;
	virtual void UploadConstant(const std::string& name, glm::vec4 constant) override;

	virtual void UploadConstant(const std::string& name, glm::mat3x3 constant) override;
	virtual void UploadConstant(const std::string& name, glm::mat4x4 constant) override;

	virtual void UploadConstant(const std::string& name, const std::vector<glm::mat4>& constant) override;

	virtual void UploadConstant(const std::string& name, int constant) override;

	virtual void UploadConstant(const std::string& name, const DirectionalLight& constant) override;
	virtual void UploadConstant(const std::string& name, const PointLight& constant) override;
	virtual void UploadConstant(const std::string& name, const SpotLight& constant) override;

	virtual void UploadConstant(const std::string& name, const DirectionalLight* constant, int count = 1) override;
	virtual void UploadConstant(const std::string& name, const PointLight* constant, int count = 1) override;
	virtual void UploadConstant(const std::string& name, const SpotLight* constant, int count = 1) override;

	virtual void UploadConstant(const std::string& name, const void* constant, unsigned long long size) override;

private:
	GLuint CompileShader(GLenum shaderType, const std::string& srcFilePath);
	std::string ReadFile(const std::string& filePath);

	std::vector<GLchar> getShaderInfoLog(GLint id);

private:
	unsigned int shaderID = 0;
};

} // namespace Hedge