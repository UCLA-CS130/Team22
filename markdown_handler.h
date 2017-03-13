#ifndef MARKDOWN_HANDLER_H
#define MARKDOWN_HANDLER_H

#include <string>
#include "request_handler.h"
#include "static_handler.h"
#include "markdown.h"

class MarkdownHandler : public StaticHandler {
protected:
	virtual bool ProcessFile(const std::string& path, const std::string& data, std::string *new_data, std::string *content_type) const;
};

REGISTER_REQUEST_HANDLER(MarkdownHandler);

#endif // MARKDOWN_HANDLER_H
