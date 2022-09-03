#include "request.h"

libevent_cpp::http_request::http_request(http_connection* conn)
    : conn_(conn) {
    kind_ = RESPONSE; // 默认是 response 
    input_buffer_ = std::unique_ptr<buffer>(new buffer());
    output_buffer_ = std::unique_ptr<buffer>(new buffer()); 
}

void libevent_cpp::http_request::reset() {
    input_buffer_->reset();
    output_buffer_->reset();
    uri_ = "";
    cb_ = nullptr;
    input_headers_.clear();
    output_headers_.clear();
    // TODO  
}

void libevent_cpp::http_request::send_error(int errorno, std::string reason) {
    std::string err_page = "<html><head>";
    err_page += "<title>" + std::to_string(errorno) + " " + reason + "</title>\n";
    err_page += "</head><body>\n";
    err_page += "<h1>Method Not Implemented</h1>\n";
    err_page += "<p>Invalid method in request</p>\n";
    err_page += "</body></html>\n";

    auto buf = std::unique_ptr<buffer>(new buffer());
    buf->push_back_string(err_page);

    input_headers_["Connection"] = "close"; // 短连接
    set_response(errorno, reason); 
    send_page(std::move(buf));
} 

void libevent_cpp::http_request::send_not_found() {
    // TODO 
    std::string not_found_page = "<html><head><title>404 Not Found</title></head>";
    not_found_page += "<body><h1>Not Found</h1>\n";
    not_found_page += "<p>The requested URL " + uri_ + " was not found on this server</p>";
    not_found_page += "</body></html>\n";

    auto buf = std::unique_ptr<buffer>(new buffer());
    buf->push_back_string(not_found_page);
    set_response(HTTP_NOT_FOUND, "Not Found"); 
    send_page(std::move(buf)); 
}

void libevent_cpp::http_request::send_page(std::unique_ptr<buffer> buf) {
    if (!http_major_version_ || !http_minor_version_) {
        http_major_version_ = http_minor_version_ = 1;
    }
    // TODO
    output_headers_.clear();
    output_headers_["Content-Type"] = "text/html; charset=utf-8";
    output_headers_["Connection"] = "close"; 

    internal_send(std::move(buf));
}

void libevent_cpp::http_request::send_reply(int code, const std::string& reason, std::unique_ptr<buffer> buf) {
    set_response(code, reason);
    internal_send(std::move(buf)); 
}

void libevent_cpp::http_request::make_header() {
    
}

void libevent_cpp::http_request::internal_send(std::unique_ptr<buffer> buf) {
    output_buffer_->push_back_buffer(buf, buf->get_cur_length());
    make_header(); 
    // TODO 
}


