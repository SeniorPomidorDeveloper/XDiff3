# -*- coding: utf-8 -*-
"""Генератор тестовых бинарников для бенчмарка xDiff3.

Создаёт:
  * базовый набор sim20/50/80/95/100 (один размер);
  * набор simXX_szNNNk_* для разных размеров файла (для выбранного similarity);
  * отдельные файлы с нулевым (разряженным) содержимым.
"""
import os, random

def make_target_from_source(source: bytes, similarity: float) -> bytes:
    """Возвращает target такой длины, что доля совпадающих байтов ~= similarity."""
    n = len(source)
    if similarity >= 0.999999:
        return bytes(source)

    src = bytearray(source)
    tgt = bytearray(src)
    # сколько байтов должны отличаться
    n_equal = int(round(n * similarity))
    n_diff = max(0, n - n_equal)
    if n_diff == 0:
        return bytes(tgt)

    indices = random.sample(range(n), n_diff)
    for i in indices:
        b = src[i]
        nb = random.randrange(0, 256)
        while nb == b:
            nb = random.randrange(0, 256)
        tgt[i] = nb
    return bytes(tgt)

def generate_basic_pairs(out_dir: str,
                         size_bytes: int = 2_000_000,
                         sims = (0.20, 0.50, 0.80, 0.95, 1.00)):
    os.makedirs(out_dir, exist_ok=True)
    # один общий source для всех sim < 1.0
    base_source = os.urandom(size_bytes)
    for sim in sims:
        tag = f"sim{int(sim*100)}"
        if sim < 1.0:
            src = base_source
            tgt = make_target_from_source(src, sim)
        else:
            src = os.urandom(size_bytes)
            tgt = bytes(src)
        with open(os.path.join(out_dir, f"{tag}_source.bin"), "wb") as f:
            f.write(src)
        with open(os.path.join(out_dir, f"{tag}_target.bin"), "wb") as f:
            f.write(tgt)
        print(f"wrote {tag}_*.bin size={size_bytes} bytes")

def generate_size_series(out_dir: str,
                         sim: float = 0.80,
                         sizes = None):
    """Файлы для графика "время vs потоки" при разных размерах файла."""
    if sizes is None:
        sizes = [256*1024, 512*1024, 1*1024*1024, 2*1024*1024, 4*1024*1024]
    os.makedirs(out_dir, exist_ok=True)
    percent = int(sim * 100)
    for size in sizes:
        src = os.urandom(size)
        tgt = make_target_from_source(src, sim)
        size_kib = size // 1024
        tag = f"sim{percent}_sz{size_kib}k"
        with open(os.path.join(out_dir, f"{tag}_source.bin"), "wb") as f:
            f.write(src)
        with open(os.path.join(out_dir, f"{tag}_target.bin"), "wb") as f:
            f.write(tgt)
        print(f"wrote {tag}_*.bin size={size} bytes (~{size/1024/1024:.2f} MiB)")


def main():
    DATA_DIR = os.getenv("DATA_DIR", "./data")
    generate_basic_pairs(DATA_DIR)
    generate_size_series(DATA_DIR)

if __name__ == "__main__":
    main()
