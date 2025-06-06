#pragma once

#include <string>
#include <vector>
#include <memory>

namespace VRDAW {

class PerformanceOptimizer {
public:
    PerformanceOptimizer();
    ~PerformanceOptimizer();

    void optimize();
    void setTargetFPS(int fps);
    void setQualityLevel(int level);
    std::string getOptimizationReport() const;

private:
    struct Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace VRDAW 