#include <vector>
#include <string>
#include "util/util_string.h"
#include "response.h"

namespace libevent_cpp {

void Response::set_header(const std::string& key, const std::string& value) {
    if (!util_string::has_crlf(key) && !util_string::has_crlf(value)) {
        headers_.emplace(key, value);
    }
}

void Response::set_content(const std::string& content, const std::string &content_type) {
    body_.assign(content);
    auto header = headers_.equal_range("Content-Type");
    headers_.erase(header.first, header.second);
    set_header("Content-Type", content_type);
}

int Response::parse_response_first_line(const std::string& data) {
    // http 响应的第一行是状态行，应该为: http-version status-code reason-phrase CRLF
    std::vector<std::string> tokens = util_string::split_string(line, ' ');
    if (tokens.size() < 3) {
        logger::error("bad http http_response status line: %s", line);
        return -1;
    }
    version_ = tokens[0];
    status_code_ = std::stoi(tokens[1]);
    status_reason_ = tokens[2];
    return 0;
}

int Response::parse_response_header(const std::string& data) {

}

}  // namespace libevent_cpp
