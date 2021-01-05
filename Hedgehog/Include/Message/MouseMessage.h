#pragma once

#include <Message/Message.h>


class MouseMessage : public Message
{
protected:
	MouseMessage(int x, int y) : x(x), y(y)
	{
		category = MessageCategory::Mouse;
	}

public:
	const int GetX() const { return x; }
	const int GetY() const { return y; }

protected:
	int x;
	int y;
};

class MouseMoveMessage : public MouseMessage
{
public:
	MouseMoveMessage(int x, int y) : MouseMessage(x, y)
	{
		type = MessageType::MouseMoved;
	}

	virtual std::string ToString() const override
	{
		std::stringstream ss;
		ss << (int)category << "::" << (int)type << " MouseMoveMessage: ";
		ss << "Position: " << x << " " << y;
		return ss.str();
	}
};

class MouseScrollMessage : public MouseMessage
{
public:
	MouseScrollMessage(int x, int y, int distance)
		: MouseMessage(x, y), distance(distance)
	{
		type = MessageType::MouseScrolled;
	}

	virtual std::string ToString() const override
	{
		std::stringstream ss;
		ss << (int)category << "::" << (int)type << " MouseScrollMessage: ";
		ss << "Scrolled " << distance << " " << x << " " << y << " ";
		return ss.str();
	}

	int GetDistance() const { return distance; }

private:
	int distance;
};
