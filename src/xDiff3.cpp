#include "xDiff3.hpp"
#include <algorithm>
#include <utility>
#include <thread>
#include <iostream>

namespace xdiff3 {

using Key32 = uint32_t;
using SrcIndex32 = std::unordered_map<Key32, std::vector<size_t>>;

static inline Key32 load_u32(const unsigned char* p) {
    return (Key32)p[0] | ((Key32)p[1] << 8) | ((Key32)p[2] << 16) | ((Key32)p[3] << 24);
}

static inline SrcIndex32 buildSourceIndex32(const ByteVec& source) {
    SrcIndex32 index;
    if (source.size() < 4) return index;
    index.reserve(source.size());
    for (size_t pos = 0; pos + 4 <= source.size(); ++pos) {
        index[load_u32(&source[pos])].push_back(pos);
    }
    return index;
}

struct Match {
    bool   found = false;
    size_t srcOffset = 0;
    size_t length = 0;
};

static inline Match findBestMatch32_segmentBounded(
    const ByteVec& source,
    const ByteVec& target,
    size_t i,
    size_t segEnd,
    const SrcIndex32& index)
{
    Match m{};
    if (i + 4 > segEnd) return m;

    auto it = index.find(load_u32(&target[i]));
    if (it == index.end()) return m;

    size_t bestLen = 0;
    size_t bestOff = 0;
    for (size_t srcOff : it->second) {
        size_t len = 4;
        while (i + len < segEnd
            && srcOff + len < source.size()
            && target[i + len] == source[srcOff + len]) {
            ++len;
        }
        if (len > bestLen) { bestLen = len; bestOff = srcOff; }
    }

    if (bestLen >= 4) {
        m.found = true;
        m.srcOffset = bestOff;
        m.length = bestLen;
    }
    return m;
}

static inline void emitInsert(std::vector<DeltaInstruction>& out, ByteVec&& buffer) {
    if (buffer.empty()) return;
    DeltaInstruction ins;
    ins.type   = DeltaInstruction::Insert;
    ins.offset = 0;
    ins.length = buffer.size();
    ins.data   = std::move(buffer);
    out.push_back(std::move(ins));
}

static inline void emitCopy(std::vector<DeltaInstruction>& out, size_t srcOffset, size_t length) {
    DeltaInstruction cp;
    cp.type   = DeltaInstruction::Copy;
    cp.offset = srcOffset;
    cp.length = length;
    cp.data.clear();
    out.push_back(std::move(cp));
}

static inline void mergeAdjacent(std::vector<DeltaInstruction>& result,
                                 std::vector<DeltaInstruction>& chunk)
{
    if (chunk.empty()) return;
    if (result.empty()) {
        result.insert(result.end(), chunk.begin(), chunk.end());
        return;
    }

    DeltaInstruction& prev  = result.back();
    DeltaInstruction& first = chunk.front();

    if (prev.type == DeltaInstruction::Insert && first.type == DeltaInstruction::Insert) {
        prev.data.insert(prev.data.end(), first.data.begin(), first.data.end());
        prev.length += first.length;
        chunk.erase(chunk.begin());
    } else if (prev.type == DeltaInstruction::Copy && first.type == DeltaInstruction::Copy) {
        if (prev.offset + prev.length == first.offset) {
            prev.length += first.length;
            chunk.erase(chunk.begin());
        }
    }
    if (!chunk.empty()) result.insert(result.end(), chunk.begin(), chunk.end());
}

std::vector<DeltaInstruction> diffParallel(
    const ByteVec& source,
    const ByteVec& target,
    unsigned numThreads)
{
    if (numThreads >= source.size() / 4) {
        numThreads = static_cast<unsigned>(source.size() / 4);
        if (numThreads == 0) numThreads = 1;
    }
    if (target.size() < 4 || numThreads < 2) {
        std::vector<DeltaInstruction> trivial(1);
        trivial[0].type = DeltaInstruction::Insert;
        trivial[0].offset = 0;
        trivial[0].length = target.size();
        trivial[0].data = target;
        return trivial;
    }

    const SrcIndex32 sourceIndex = buildSourceIndex32(source);

    const size_t totalSize   = target.size();
    const size_t segmentSize = (totalSize + numThreads - 1) / numThreads; // ceil
    std::vector<std::vector<DeltaInstruction>> threadResults(numThreads);
    std::vector<std::thread> threads;
    threads.reserve(numThreads);

    for (unsigned t = 0; t < numThreads; ++t) {
        size_t segStart = t * segmentSize;
        if (segStart >= totalSize) break;
        size_t segEnd = std::min(totalSize, segStart + segmentSize);

        threads.emplace_back([&, t, segStart, segEnd]() {
            std::vector<DeltaInstruction> localInstr;
            ByteVec insertBuffer;
            insertBuffer.reserve(64);

            size_t i = segStart;
            while (i < segEnd) {
                if (i + 4 > segEnd) break;

                Match m = findBestMatch32_segmentBounded(source, target, i, segEnd, sourceIndex);
                if (m.found) {
                    emitInsert(localInstr, std::move(insertBuffer));
                    insertBuffer.clear();
                    emitCopy(localInstr, m.srcOffset, m.length);
                    i += m.length;
                } else {
                    insertBuffer.push_back(target[i]);
                    ++i;
                }
            }

            if (i < segEnd) {
                insertBuffer.insert(insertBuffer.end(), target.begin() + i, target.begin() + segEnd);
            }
            emitInsert(localInstr, std::move(insertBuffer));

            threadResults[t] = std::move(localInstr);
        });
    }

    for (auto& th : threads) th.join();

    std::vector<DeltaInstruction> result;
    result.reserve(target.size() / 3 + 8);
    for (unsigned t = 0; t < numThreads; ++t) {
        if (t >= threadResults.size()) break;
        auto& chunk = threadResults[t];
        if (chunk.empty()) continue;
        mergeAdjacent(result, chunk);
    }
    return result;
}

} // namespace xdiff3
