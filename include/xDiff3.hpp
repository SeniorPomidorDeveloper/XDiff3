#pragma once
#include <vector>
#include <thread>
#include <unordered_map>
#include <cstddef>

namespace xdiff3 {

struct DeltaInstruction {
    enum Type { Copy, Insert } type;
    size_t offset;                   // для Copy: смещение в source; для Insert — 0
    size_t length;                   // длина сегмента
    std::vector<unsigned char> data; // для Insert; для Copy — пусто
};

using ByteVec = std::vector<unsigned char>;

// Параллельный дельта-алгоритм (keyLen=4, индекс по 4-байтовым ключам).
// Возвращает поток инструкций COPY/INSERT, чтобы восстановить target из source.
std::vector<DeltaInstruction> diffParallel(
    const ByteVec& source,
    const ByteVec& target,
    unsigned numThreads = std::thread::hardware_concurrency());

} // namespace xdiff3
