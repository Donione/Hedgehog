#pragma once

#include <Renderer/Shader.h>

class OpenGLShader : public Shader
{
public:
	OpenGLShader(const std::string& vertexFilePath, const std::string& pixelFilePath);
	virtual ~OpenGLShader() override;

	virtual void Bind() const override;
	virtual void Unbind() const override;

private:
	std::string ReadFile(const std::string& filePath);

private:
	unsigned int rendererID = 0;
};