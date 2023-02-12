#include "base/epoll_base.h"
#include "client.h"

namespace libevent_cpp {

http_client::http_client(const std::string& host)
    : http_client(host, 443, std::string(), std::string()) {}

http_client::http_client(const std::string& host, uint16_t port)
    : http_client(host, port, std::string(), std::string()) {}

http_client::http_client(const std::string& host, uint16_t port,
    const std::string& client_cert_path, const std::string& client_key_path)
    : http_client_impl(host, port, client_cert_path, client_key_path) {
    ctx_ = SSL_CTX_new(TLS_client_method());
    if (!client_cert_path.empty() && !client_key_path.empty()) {
        if (SSL_CTX_use_certificate_file(ctx_, client_cert_path.c_str(), SSL_FILETYPE_PEM) != 1
            || SSL_CTX_use_PrivateKey_file(ctx_, client_key_path.c_str(), SSL_FILETYPE_PEM) != 1) {
            SSL_CTX_free(ctx_);
            ctx_ = nullptr;
        }
    }
}

http_client::http_client(const std::string& host, uint16_t port, X509* client_cert, EVP_PKEY* client_key)
    : http_client_impl(host, port) {
    ctx_ = SSL_CTX_new(TLS_client_method());
    if (client_cert != nullptr && client_key != nullptr) {
        if (SSL_CTX_use_certificate(ctx_, client_cert) != 1
        || SSL_CTX_use_PrivateKey(ctx_, client_key) != 1) {
            SSL_CTX_free(ctx_);
            ctx_ = nullptr;
        }
    }
}

http_client::~http_client() {
    if (ctx_) {
        SSL_CTX_free(ctx_);
    }
}

void http_client::set_ca_cert_store(X509_STORE* ca_cert_store) {
    if (ca_cert_store) {
        if (ctx_) {
            if (SSL_CTX_get_cert_store(ctx_) != ca_cert_store) {
                SSL_CTX_set_cert_store(ctx_, ca_cert_store);
            }
        } else {
            X509_STORE_free(ca_cert_store);
        }
    }
}

// http_client::http_client(const std::string& host, uint16_t port)
//     : server_host_(host), server_port_(port) {
//     // 为了实现异步接口
//     event_manager_ = std::make_shared<epoll_base>();
//     // http 连接
//     http_connection_ = std::make_shared<http_connection>(event_manager_, host, port);
// }

// http_client_result http_client::Get(const std::string& path) {
//     return Get(path, Headers(), Progress());
// }

// http_client_result http_client::Get(const std::string& path, const Headers& headers) {
//     return Get(path, headers, Progress());
// }

// http_client_result http_client::Get(const std::string& path, const Headers& headers, Progress progress) {
//     http_request req;
//     req.set_request_method("GET");
//     req.set_request_path(path);
//     req.set_request_headers(headers);
//     req.set_request_progress(std::move(progress));
    
// }

// void http_client::GetAsync(const std::string& path, http_client_result_cb cb) {
//     std::unique_ptr<http_request> req(new http_request());
//     req->set_request_method("GET");
//     req->set_request_path(path);
//     req->set_client_result_cb(cb);
//     http_connection_->create_request(std::move(req));
//     event_manager_dispatch();
// }

// void http_client::event_manager_dispatch() {
//     // 局部静态变量只会初始化一次
//     static auto once_call = [&]() {
//         if (event_manager_) {
//             event_manager_->start_dispatch();
//         }
//     }();
// }

}  // namespace libevent_cpp
