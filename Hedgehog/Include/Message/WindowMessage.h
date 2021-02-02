#pragma once

#include <Message/Message.h>


class WindowSizeMessage : public Message
{
public:
	WindowSizeMessage(unsigned int width, unsigned int height)
		: width(width), height(height)
	{
		category = MessageCategory::Window;
		type = MessageType::WindowSize;
	}

	unsigned int GetWidth() const { return width; }
	unsigned int GetHeight() const { return height; }

	virtual std::string ToString() const override
	{
		std::stringstream ss;
		ss << (int)category << "::" << (int)type << " WindowSizeMessage: ";
		ss << "Window size (client area): " << width << "x" << height << "\n";
		return ss.str();
	}

private:
	// Size of the client area of the window
	unsigned int width;
	unsigned int height;
};


class WindowPositionMessage : public Message
{
public:
	WindowPositionMessage(int right, int bottom)
		: left(0), top(0), right(right), bottom(bottom)
	{
		category = MessageCategory::Window;
		type = MessageType::WindowPosition;
	}
	WindowPositionMessage(int left, int top, int right, int bottom)
		: left(left), top(top), right(right), bottom(bottom)
	{
		category = MessageCategory::Window;
		type = MessageType::WindowPosition;
	}

	virtual std::string ToString() const override
	{
		std::stringstream ss;
		ss << (int)category << "::" << (int)type << " WindowPositionMessage: ";
		ss << "Window position: " << "(" << left << ", " << top << "), (" << right << ", " << bottom << ")\n";
		return ss.str();
	}

protected:
	// Window position releative to the top-left corner of the screen
	int left;
	int top;
	int right;
	int bottom;
};
