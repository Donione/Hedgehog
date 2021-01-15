#pragma once

#include <Renderer/Texture.h>


class OpenGLTexture2D : public Texture2D
{
public:
	OpenGLTexture2D(const std::string& filename);
	virtual ~OpenGLTexture2D();

	virtual void Bind(unsigned int slot = 0) const override;

	virtual unsigned int GetWidth() const override { return width; }
	virtual unsigned int GetHeight() const override { return height; }

private:
	unsigned int textureID = 0;

	std::string filename;

	unsigned int width;
	unsigned int height;
};
