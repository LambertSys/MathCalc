#pragma once

#include <string>
#include <vector>

#include "SourceLocation.hpp"

enum class ReportType
{
	Error,
	Warning,
	Info
};

struct OutMessage
{
	ReportType type;
	std::string message;

	SourceLocation loc;

	// для ошибок типо линковщика, где нет точного места в коде
	bool messageInSource;
};

using MessageList = std::vector<OutMessage>;

class Report
{
public:
	static void PushError(std::string message, SourceLocation loc);
	static void PushWarning(std::string message, SourceLocation loc);
	static void PushInfo(std::string message, SourceLocation loc);

	static MessageList& GetMessages();
};
