#pragma once

#include <string>
#include <vector>
#include <functional>

namespace VR_DAW {

class NetworkManager {
public:
    NetworkManager();
    ~NetworkManager();

    void connect(const std::string& url);
    void disconnect();
    void sendMessage(const std::string& message);
    std::vector<std::string> getMessages();
};

} // namespace VR_DAW 