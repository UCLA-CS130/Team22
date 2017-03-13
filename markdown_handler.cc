#include <sstream>
#include "markdown_handler.h"

bool MarkdownHandler::ProcessFile(const std::string& path, const std::string& data, std::string *new_data, std::string *content_type) const {

	std::ostringstream out;
	markdown::Document doc;
	doc.read(data);
	doc.write(out);

	*new_data = out.str();
	*content_type = "text/html";

	return true;
}
