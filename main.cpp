#include "include/xDiff3.hpp"
#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <chrono>
#include <thread>
#include <cstdint>

// ==== утилиты ввода/вывода файлов ====

static std::vector<unsigned char> read_file(const std::string& path) {
    std::ifstream f(path, std::ios::binary | std::ios::ate);
    if (!f) throw std::runtime_error("Cannot open file: " + path);
    const std::streamsize size = f.tellg();
    std::vector<unsigned char> buf(static_cast<size_t>(size));
    f.seekg(0);
    if (size > 0) f.read(reinterpret_cast<char*>(buf.data()), size);
    return buf;
}

static void write_patch(const std::string& filename,
                        const std::vector<xdiff3::DeltaInstruction>& prog)
{
    std::ofstream out(filename, std::ios::binary);
    if (!out) throw std::runtime_error("Cannot create patch file: " + filename);

    // (необязательно) можно записать магическую сигнатуру и версию формата
    const char magic[8] = {'X','D','3','P','A','T','C','H'};
    out.write(magic, sizeof(magic));
    uint32_t version = 1;
    out.write(reinterpret_cast<const char*>(&version), sizeof(version));

    for (const auto& ins : prog) {
        uint8_t op = (ins.type == xdiff3::DeltaInstruction::Copy) ? 0u : 1u;
        out.write(reinterpret_cast<const char*>(&op), sizeof(op));
        if (op == 0u) {
            // COPY: op + offset + length
            out.write(reinterpret_cast<const char*>(&ins.offset), sizeof(ins.offset));
            out.write(reinterpret_cast<const char*>(&ins.length), sizeof(ins.length));
        } else {
            // INSERT: op + length + data[length]
            out.write(reinterpret_cast<const char*>(&ins.length), sizeof(ins.length));
            if (ins.length) {
                out.write(reinterpret_cast<const char*>(ins.data.data()),
                          static_cast<std::streamsize>(ins.length));
            }
        }
    }
    // (необязательно) хвостовая контрольная сумма/размеры могут быть добавлены здесь
}

// ==== печать статистики (по желанию) ====

static void print_stats(const std::vector<xdiff3::DeltaInstruction>& prog,
                        size_t src_size, size_t tgt_size, double ms, unsigned threads)
{
    size_t copies=0, inserts=0, copy_bytes=0, insert_bytes=0;
    size_t est_patch_size=0;
    for (const auto& ins : prog) {
        if (ins.type == xdiff3::DeltaInstruction::Copy) {
            ++copies; copy_bytes += ins.length;
            est_patch_size += 1 + sizeof(size_t)*2;
        } else {
            ++inserts; insert_bytes += ins.length;
            est_patch_size += 1 + sizeof(size_t) + ins.data.size();
        }
    }

    std::cout << "Source size: " << src_size << " bytes\n";
    std::cout << "Target size: " << tgt_size << " bytes\n";
    std::cout << "Instructions: " << prog.size()
              << " (COPY=" << copies << "/" << copy_bytes << "B, "
              << "INSERT=" << inserts << "/" << insert_bytes << "B)\n";
    std::cout << "Estimated patch size: " << est_patch_size << " bytes "
              << "(" << std::fixed << std::setprecision(2)
              << (tgt_size ? (100.0 * est_patch_size / tgt_size) : 0.0) << "% of target)\n";
    std::cout << "Threads used: " << threads << "\n";
    std::cout << "Build time: " << std::fixed << std::setprecision(3) << ms << " ms\n";
}

int main(int argc, char** argv) {
    try {
        if (argc < 3 || argc > 5) {
            std::cerr << "Usage: " << argv[0]
                      << " <source_file> <target_file> [threads] [patch_out=delta.patch]\n";
            return 2;
        }
        const std::string srcPath = argv[1];
        const std::string tgtPath = argv[2];

        unsigned threads = std::thread::hardware_concurrency();
        if (argc >= 4) threads = static_cast<unsigned>(std::stoul(argv[3]));
        if (threads == 0) threads = 1;

        std::string patch_out = "delta.patch";
        if (argc == 5) patch_out = argv[4];

        const auto source = read_file(srcPath);
        const auto target = read_file(tgtPath);

        const auto start = std::chrono::high_resolution_clock::now();
        auto program = xdiff3::diffParallel(source, target, threads);
        const auto stop  = std::chrono::high_resolution_clock::now();
        const double ms = std::chrono::duration<double, std::milli>(stop - start).count();

        // записываем результат
        write_patch(patch_out, program);
        std::cout << "Patch written: " << patch_out << "\n";

        // печать сводки (опционально, можно убрать)
        print_stats(program, source.size(), target.size(), ms, threads);

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}
