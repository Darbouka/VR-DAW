#pragma once
#include <string>

namespace VR_DAW {

class BackendServer {
public:
    BackendServer();
    ~BackendServer();

    void start(int port = 8080);
    void stop();
    std::string handleRequest(const std::string& endpoint, const std::string& data);

private:
    struct Impl;
    Impl* pImpl;
};

} // namespace VR_DAW 