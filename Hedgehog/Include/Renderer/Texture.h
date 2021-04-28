#pragma once

#include <string>


namespace Hedge
{

enum class TextureType
{
	Diffuse,
	Specular,
	Normal,
	Generic
};

struct TextureDescription
{
	TextureType type = TextureType::Generic;
	std::string filename;

	TextureDescription() = default;
	TextureDescription(TextureType type, const std::string& filename = "") : type(type), filename(filename) {}
};


class Texture
{
public:
	virtual ~Texture() = default;

	virtual void Bind(unsigned int slot = 0) const = 0;

	virtual unsigned int GetWidth() const = 0;
	virtual unsigned int GetHeight() const = 0;
};


class Texture2D : public Texture
{
public:
	static Texture2D* Create(const std::string& filename);
};

} // namespace Hedge
