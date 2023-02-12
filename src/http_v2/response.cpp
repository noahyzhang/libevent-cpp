#include <vector>
#include <string>
#include <regex>
#include "util/util_string.h"
#include "response.h"

namespace libevent_cpp {

int http_response::set_resp_first_line(const std::string& first_line) {
    static const std::regex reg("(HTTP/1\\.[01]) (\\d{3})(?: (.*?))?\r?\n");
    std::cmatch cm;
    if (!std::regex_match(first_line.data(), cm, reg)) {
        return -1;
    }
    set_version(std::string(cm[1]));
    set_status_code(std::stoi(std::string(cm[2])));
    set_status_reason(std::string(cm[3]));
    return 0;
}

void http_response::set_header(const std::string& key, const std::string& value) {
    if (!util_string::has_crlf(key) && !util_string::has_crlf(value)) {
        headers_.emplace(key, value);
    }
}

void http_response::set_header(const std::string& line_data) {
    auto pos = line_data.find_first_of(':');
    if (pos == std::string::npos) {
        return;
    }
    std::string key = line_data.substr(0, pos-1);
    std::string val = line_data.substr(pos+1);
    headers_.emplace(key, val);
}

void http_response::set_content(const std::string& content, const std::string &content_type) {
    body_.assign(content);
    auto header = headers_.equal_range("Content-Type");
    headers_.erase(header.first, header.second);
    set_header("Content-Type", content_type);
}

int http_response::parse_response_first_line(const std::string& data) {
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

int http_response::parse_response_header(const std::string& data) {

}

}  // namespace libevent_cpp
