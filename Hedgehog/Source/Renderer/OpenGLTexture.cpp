#include <Renderer/OpenGLTexture.h>

#include <glad/glad.h>
#include <stb_image.h>


namespace Hedge
{

OpenGLTexture2D::OpenGLTexture2D(const std::string& filename)
	: filename(filename)
{
	int width;
	int height;
	int channels;

	stbi_set_flip_vertically_on_load(1);
	stbi_uc* data = stbi_load(filename.c_str(), &width, &height, &channels, 0);

	this->width = width;
	this->height = height;

	GLenum internalFormat = GL_RGB8;
	GLenum dataFormat = GL_RGB;
	if (channels == STBI_rgb)
	{
		internalFormat = GL_RGB8;
		dataFormat = GL_RGB;
	}
	else if (channels == STBI_rgb_alpha)
	{
		internalFormat = GL_RGBA8;
		dataFormat = GL_RGBA;
	}

	glCreateTextures(GL_TEXTURE_2D, 1, &textureID);
	glTextureStorage2D(textureID, 1, internalFormat, width, height);

	glTextureParameteri(textureID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTextureParameteri(textureID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glTextureSubImage2D(textureID, 0, 0, 0, width, height, dataFormat, GL_UNSIGNED_BYTE, data);

	stbi_image_free(data);
}

OpenGLTexture2D::~OpenGLTexture2D()
{
	glDeleteTextures(1, &textureID);
}

void OpenGLTexture2D::Bind(unsigned int slot) const
{
	glBindTextureUnit(slot, textureID);
}

} // namespace Hedge
