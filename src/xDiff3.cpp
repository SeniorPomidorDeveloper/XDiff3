#include "xDiff3.hpp"
#include <unordered_map>
#include <string>
#include <thread>
#include <algorithm>

namespace xdiff3 {

// Константа длины ключа для индексации
static constexpr std::size_t KEY_LENGTH = 4;

// Тип индекса: ключ -> список позиций в source
using IndexMap = std::unordered_map<std::string, std::vector<std::size_t>>;

// 1. Построение индекса 4-байтных ключей из source
static IndexMap buildIndex(const std::vector<unsigned char>& source) {
    IndexMap index;
    if (source.size() >= KEY_LENGTH) {
        index.reserve(source.size());
        for (std::size_t i = 0; i + KEY_LENGTH <= source.size(); ++i) {
            std::string key(reinterpret_cast<const char*>(&source[i]), KEY_LENGTH);
            index[key].push_back(i);
        }
    }
    return index;
}

// 2. Поиск наилучшего совпадения для позиции в target
// Возвращает: {offset, length} или {0, 0} если совпадения нет
static std::pair<std::size_t, std::size_t> findBestMatch(
    const IndexMap& index,
    const std::vector<unsigned char>& source,
    const std::vector<unsigned char>& target,
    std::size_t targetPos,
    std::size_t segmentEnd)
{
    if (targetPos + KEY_LENGTH > segmentEnd || index.empty()) {
        return {0, 0};
    }

    std::string key(reinterpret_cast<const char*>(&target[targetPos]), KEY_LENGTH);
    auto it = index.find(key);
    if (it == index.end()) {
        return {0, 0};
    }

    std::size_t bestLen = KEY_LENGTH;
    std::size_t bestOff = it->second[0];

    for (std::size_t sourceOff : it->second) {
        std::size_t len = KEY_LENGTH;
        while (targetPos + len < segmentEnd && 
               sourceOff + len < source.size() &&
               target[targetPos + len] == source[sourceOff + len]) {
            ++len;
        }
        if (len > bestLen) {
            bestLen = len;
            bestOff = sourceOff;
        }
    }

    return {bestOff, bestLen};
}

// 3. Обработка одного сегмента target (выполняется в отдельном потоке)
static std::vector<DeltaInstruction> processSegment(
    const IndexMap& index,
    const std::vector<unsigned char>& source,
    const std::vector<unsigned char>& target,
    std::size_t start,
    std::size_t end)
{
    std::vector<DeltaInstruction> instructions;
    std::vector<unsigned char> insertBuffer;
    std::size_t i = start;

    while (i < end) {
        auto [offset, length] = findBestMatch(index, source, target, i, end);
        
        if (length >= KEY_LENGTH) {
            // Найдено совпадение - сохраняем накопленный INSERT (если есть)
            if (!insertBuffer.empty()) {
                instructions.push_back({
                    DeltaInstruction::Insert, 
                    0, 
                    insertBuffer.size(), 
                    insertBuffer
                });
                insertBuffer.clear();
            }
            // Добавляем COPY инструкцию
            instructions.push_back({
                DeltaInstruction::Copy, 
                offset, 
                length, 
                {}
            });
            i += length;
        } else {
            // Совпадения нет - накапливаем байт для INSERT
            insertBuffer.push_back(target[i]);
            ++i;
        }
    }

    // Добавляем остаток сегмента (если обработка прервалась из-за границы)
    if (i < end) {
        insertBuffer.insert(insertBuffer.end(), 
                          target.begin() + i, 
                          target.begin() + end);
    }

    // Финальный INSERT (если есть накопленные данные)
    if (!insertBuffer.empty()) {
        instructions.push_back({
            DeltaInstruction::Insert, 
            0, 
            insertBuffer.size(), 
            insertBuffer
        });
    }

    return instructions;
}

// 4. Слияние результатов из всех потоков
static std::vector<DeltaInstruction> mergeResults(
    std::vector<std::vector<DeltaInstruction>>& parts)
{
    std::vector<DeltaInstruction> result;

    for (auto& chunk : parts) {
        if (chunk.empty()) continue;

        // Слияние соседних INSERT на границах сегментов
        if (!result.empty() && 
            result.back().type == DeltaInstruction::Insert &&
            chunk.front().type == DeltaInstruction::Insert) {
            
            result.back().data.insert(
                result.back().data.end(),
                chunk.front().data.begin(), 
                chunk.front().data.end()
            );
            result.back().length += chunk.front().length;
            chunk.erase(chunk.begin());
        }

        result.insert(result.end(), chunk.begin(), chunk.end());
    }

    return result;
}

// 5. Главная функция параллельной обработки (упрощенная)
static std::vector<DeltaInstruction> diffParallelImpl(
    const std::vector<unsigned char>& source,
    const std::vector<unsigned char>& target,
    unsigned numThreads)
{
    // Построение индекса
    IndexMap index = buildIndex(source);

    // Подготовка параллелизации
    if (numThreads < 1) numThreads = 1;
    const std::size_t targetSize = target.size();
    const std::size_t segmentSize = std::max<std::size_t>(
        KEY_LENGTH, 
        (targetSize + numThreads - 1) / numThreads
    );

    // Контейнеры для результатов и потоков
    std::vector<std::vector<DeltaInstruction>> parts(numThreads);
    std::vector<std::thread> threads;
    threads.reserve(numThreads);

    // Запуск потоков для обработки сегментов
    for (unsigned t = 0; t < numThreads; ++t) {
        const std::size_t start = t * segmentSize;
        if (start >= targetSize) break;
        const std::size_t end = std::min(targetSize, start + segmentSize);

        threads.emplace_back([&, t, start, end]() {
            parts[t] = processSegment(index, source, target, start, end);
        });
    }

    // Ожидание завершения всех потоков
    for (auto& thread : threads) {
        thread.join();
    }

    // Слияние результатов
    return mergeResults(parts);
}

std::vector<DeltaInstruction> diffParallel(
    const std::vector<unsigned char>& source,
    const std::vector<unsigned char>& target,
    unsigned numThreads)
{
    return diffParallelImpl(source, target, numThreads);
}

} // namespace xdiff3
