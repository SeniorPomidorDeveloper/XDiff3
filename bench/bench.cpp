#include "xDiff3.hpp"
#include <vector>
#include <chrono>
#include <algorithm>
using namespace xdiff3;

extern "C"
double xdiff3_diff_time_from_buffers(const uint8_t* src, std::size_t src_size,
                                     const uint8_t* tgt, std::size_t tgt_size,
                                     int numThreads)
{
    if (!src || !tgt || src_size == 0 || tgt_size == 0) return 0.0;
    std::vector<unsigned char> source(src, src + src_size);
    std::vector<unsigned char> target(tgt, tgt + tgt_size);

    auto t0 = std::chrono::high_resolution_clock::now();
    auto patch = diffParallel(source, target, (unsigned)std::max(1, numThreads));
    (void)patch;
    auto t1 = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double>(t1 - t0).count();
}
