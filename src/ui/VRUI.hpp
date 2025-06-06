#pragma once
#include <memory>

namespace VR_DAW {

class VRUI {
public:
    static VRUI& getInstance();
    bool initialize();
    void shutdown();
    void render();
    void handleInput();
    void setScale(float scale);

private:
    VRUI();
    ~VRUI();
    VRUI(const VRUI&) = delete;
    VRUI& operator=(const VRUI&) = delete;
    struct Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace VR_DAW 