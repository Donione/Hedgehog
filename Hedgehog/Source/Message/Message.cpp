#include <Message/Message.h>


namespace Hedge
{

MessageCategory Message::GetMessageCategory() const
{
	return category;
}

MessageType Message::GetMessageType() const
{
	return type;
}

bool Message::IsInClass(MessageCategory messageClass) const
{
	return GetMessageCategory() == messageClass;
}

} // namespace Hedge
