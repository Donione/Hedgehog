#pragma once

#include <Message/Message.h>


class KeyMessage : public Message
{
public:
	KeyMessage(const unsigned char virtualKeyCode,
			   const unsigned short repeatCount,
			   const bool previousState)
		:
		virtualKeyCode(virtualKeyCode),
		repeatCount(repeatCount),
		previousState(previousState)
	{
		category = MessageCategory::Key;
	}

protected:
	unsigned char virtualKeyCode;
	unsigned short repeatCount;
	bool previousState;
};

class KeyPressedMessage : public KeyMessage
{
public:
	KeyPressedMessage(const unsigned char virtualKeyCode,
					  const unsigned short repeatCount,
					  const bool previousState)
		:
		KeyMessage(virtualKeyCode, repeatCount, previousState)
	{
		type = MessageType::KeyPressed;
	}

	std::string ToString() const override
	{
		std::stringstream ss;
		ss << (int)category << "::" << (int)type << " KeyPressedMessage: ";// << virtualKeyCode;
		ss << "0x" << std::uppercase << std::setfill('0') << std::setw(2) << std::hex << (unsigned int)virtualKeyCode;
		if (previousState)
		{
			ss << "(" << repeatCount << ")";
		}
		return ss.str();

	}
};

class KeyReleasedMessage : public KeyMessage
{
public:
	KeyReleasedMessage(const unsigned char virtualKeyCode,
					  const unsigned short repeatCount,
					  const bool previousState)
		:
		KeyMessage(virtualKeyCode, repeatCount, previousState)
	{
		type = MessageType::KeyReleased;
	}

	std::string ToString() const override
	{
		std::stringstream ss;
		ss << (int)category << "::" << (int)type << " KeyReleasedMessage: ";// << virtualKeyCode;
		ss << "0x" << std::uppercase << std::setfill('0') << std::setw(2) << std::hex << (unsigned int)virtualKeyCode;
		return ss.str();

	}
};