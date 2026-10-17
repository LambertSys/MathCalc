#include "Report.hpp"

static std::vector<OutMessage> messages;

void Report::PushError(std::string message, SourceLocation loc)
{
	messages.push_back(OutMessage{ .type = ReportType::Error, .message = message, .loc = loc, .messageInSource = true });
}

void Report::PushWarning(std::string message, SourceLocation loc)
{
	messages.push_back(OutMessage{ .type = ReportType::Warning, .message = message, .loc = loc, .messageInSource = true });
}

void Report::PushInfo(std::string message, SourceLocation loc)
{
	messages.push_back(OutMessage{ .type = ReportType::Info, .message = message, .loc = loc, .messageInSource = true });
}

MessageList& Report::GetMessages()
{
	return messages;
}
