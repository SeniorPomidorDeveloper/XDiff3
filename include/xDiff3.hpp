#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>

namespace xdiff3 {

struct DeltaInstruction {
    enum Type { Copy, Insert } type;
    std::size_t offset;
    std::size_t length;
    std::vector<unsigned char> data;
};

// Основное API: именно это вызывает твой main.cpp
std::vector<DeltaInstruction> diffParallel(
    const std::vector<unsigned char>& source,
    const std::vector<unsigned char>& target,
    unsigned numThreads);

} // namespace xdiff3
