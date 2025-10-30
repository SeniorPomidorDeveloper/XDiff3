#include "xDiff3.hpp"
#include <unordered_map>
#include <string>
#include <thread>
#include <algorithm>

namespace xdiff3 {

static std::vector<DeltaInstruction> diffParallelImpl(
    const std::vector<unsigned char>& source,
    const std::vector<unsigned char>& target,
    unsigned numThreads)
{
    const std::size_t keyLen = 4;
    std::unordered_map<std::string, std::vector<std::size_t>> index;
    if (source.size() >= keyLen) {
        index.reserve(source.size());
        for (std::size_t i = 0; i + keyLen <= source.size(); ++i) {
            std::string k(reinterpret_cast<const char*>(&source[i]), keyLen);
            index[k].push_back(i);
        }
    }

    if (numThreads < 1) numThreads = 1;
    const std::size_t n = target.size();
    const std::size_t segSz = std::max<std::size_t>(keyLen, (n + numThreads - 1) / numThreads);

    std::vector<std::vector<DeltaInstruction>> parts(numThreads);
    std::vector<std::thread> ths;
    ths.reserve(numThreads);

    for (unsigned t = 0; t < numThreads; ++t) {
        const std::size_t start = t * segSz;
        if (start >= n) break;
        const std::size_t end = std::min(n, start + segSz);

        ths.emplace_back([&, t, start, end](){
            std::vector<DeltaInstruction> local;
            std::vector<unsigned char> insertBuf;
            std::size_t i = start;

            while (i < end) {
                if (i + keyLen > end || index.empty()) break;
                std::string k(reinterpret_cast<const char*>(&target[i]), keyLen);
                auto it = index.find(k);
                if (it != index.end()) {
                    std::size_t bestLen = keyLen, bestOff = it->second[0];
                    for (std::size_t off : it->second) {
                        std::size_t len = keyLen;
                        while (i + len < end && off + len < source.size()
                               && target[i+len] == source[off+len]) ++len;
                        if (len > bestLen) { bestLen = len; bestOff = off; }
                    }
                    if (!insertBuf.empty()) {
                        local.push_back({DeltaInstruction::Insert, 0, insertBuf.size(), insertBuf});
                        insertBuf.clear();
                    }
                    local.push_back({DeltaInstruction::Copy, bestOff, bestLen, {}});
                    i += bestLen;
                } else {
                    insertBuf.push_back(target[i]);
                    ++i;
                }
            }
            if (i < end)
                insertBuf.insert(insertBuf.end(), target.begin()+i, target.begin()+end);
            if (!insertBuf.empty())
                local.push_back({DeltaInstruction::Insert, 0, insertBuf.size(), insertBuf});
            parts[t] = std::move(local);
        });
    }
    for (auto& th : ths) th.join();

    std::vector<DeltaInstruction> out;
    for (unsigned t = 0; t < numThreads; ++t) {
        if (t >= parts.size()) break;
        auto& chunk = parts[t];
        if (chunk.empty()) continue;
        if (!out.empty() && out.back().type == DeltaInstruction::Insert &&
            chunk.front().type == DeltaInstruction::Insert) {
            out.back().data.insert(out.back().data.end(),
                                   chunk.front().data.begin(), chunk.front().data.end());
            out.back().length += chunk.front().length;
            chunk.erase(chunk.begin());
        }
        out.insert(out.end(), chunk.begin(), chunk.end());
    }
    return out;
}

std::vector<DeltaInstruction> diffParallel(
    const std::vector<unsigned char>& source,
    const std::vector<unsigned char>& target,
    unsigned numThreads)
{
    return diffParallelImpl(source, target, numThreads);
}

} // namespace xdiff3
