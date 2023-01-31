// Copyright 2022 Tencent LLC
// Author: noahyzhang

#pragma once

#include <string>
#include "common.h"

namespace libevent_cpp {

class http_client_result {

};

class http_client {
public:
    http_client_result Get(const std::string& path);
    http_client_result Get(const std::string& path, const Headers& headers);

    http_client_result Head(const std::string& path);
    http_client_result Head(const std::string& path, const Headers& headers);

    http_client_result Post(const std::string& path);
    http_client_result Post(const std::string& path, const Headers& headers);

    http_client_result Put(const std::string& path);

    http_client_result Patch(const std::string& path);

    http_client_result Delete(const std::string& path);

};

}  // namespace libevent_cpp
