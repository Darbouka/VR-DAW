#include "BackendServer.hpp"
#include <iostream>

namespace VR_DAW {

struct BackendServer::Impl {
    bool running = false;
    int port = 8080;
};

BackendServer::BackendServer() : pImpl(new Impl) {}
BackendServer::~BackendServer() { delete pImpl; }

void BackendServer::start(int port) {
    pImpl->running = true;
    pImpl->port = port;
    std::cout << "Backend-Server gestartet auf Port " << port << std::endl;
}

void BackendServer::stop() {
    pImpl->running = false;
    std::cout << "Backend-Server gestoppt." << std::endl;
}

std::string BackendServer::handleRequest(const std::string& endpoint, const std::string& data) {
    // TODO: REST-API-Logik
    std::cout << "Request: " << endpoint << " | Data: " << data << std::endl;
    return "{\"status\":\"ok\"}";
}

} // namespace VR_DAW 