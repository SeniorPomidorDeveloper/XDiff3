import os, csv, glob
from datetime import datetime
from core_ctypes import load_lib, run_from_buffers
from metrics import aggregate, calc_time_error
from plots_v2 import plot_time_by_size, plot_time_by_size_err

def read_file(path: str) -> bytes:
    with open(path, "rb") as f:
        return f.read()

def env_list(name, cast, default):
    txt = os.getenv(name, "")
    if not txt:
        return default
    return [cast(x) for x in txt.replace(",", " ").split() if x.strip()]

def discover_pairs_by_size(data_dir: str, similarity: float):
    """Ищем пары файлов для одного similarity, но разных размеров.

    Ожидаемый паттерн имён:
        sim80_szXXXXk_source.bin / sim80_szXXXXk_target.bin
    """
    percent = int(round(similarity * 100))
    pattern = os.path.join(data_dir, f"sim{percent}_sz*_source.bin")
    pairs = {}
    for src_path in sorted(glob.glob(pattern)):
        base = os.path.basename(src_path).split("_source.bin")[0]
        tgt_path = os.path.join(data_dir, f"{base}_target.bin")
        if os.path.exists(tgt_path):
            size_bytes = os.path.getsize(src_path)
            pairs[size_bytes] = (src_path, tgt_path)
    return pairs

def main():
    LIB_PATH = os.getenv("LIB_PATH", "../build/libxdiff3_bench.so")
    DATA_DIR = os.getenv("DATA_DIR", "./data")
    THREADS  = env_list("THREADS_LIST", int, list(range(1, 41)))
    ATTEMPTS = int(os.getenv("ATTEMPTS", "5"))
    OUT_DIR  = os.getenv("OUT_DIR", ".")
    SIM      = float(os.getenv("SIM_FOR_SIZES", "0.80"))  # одно similarity для этого эксперимента

    pairs = discover_pairs_by_size(DATA_DIR, SIM)
    if not pairs:
        raise SystemExit(f"Не найдены пары файлов sim{int(SIM*100)}_sz* в {DATA_DIR}")

    lib = load_lib(LIB_PATH)
    os.makedirs(OUT_DIR, exist_ok=True)

    times = {size: {p: [] for p in THREADS} for size in sorted(pairs.keys())}

    buffers = {size: (read_file(pairs[size][0]), read_file(pairs[size][1]))
               for size in pairs}

    for size in sorted(pairs.keys()):
        src, tgt = buffers[size]
        for p in THREADS:
            for _ in range(ATTEMPTS):
                dt = run_from_buffers(lib, src, tgt, p)
                times[size][p].append(dt)

    med_time = {}
    time_err = {}
    for size in sorted(times.keys()):
        med, _, _ = aggregate(times[size])
        med_time[size] = med
        time_err[size] = calc_time_error(times[size])

    ts = datetime.now().strftime("%Y%m%d_%H%M%S")
    csv_path = os.path.join(OUT_DIR, f"xdiff3_sizes_sim{int(SIM*100)}_{ts}.csv")
    with open(csv_path, "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["size_bytes","threads","time_med_s"])
        for size in sorted(times.keys()):
            for p in THREADS:
                w.writerow([size, p, f"{med_time[size][p]:.9f}"])

    png_base = os.path.join(OUT_DIR, f"time_size_sim{int(SIM*100)}_{ts}")
    plot_time_by_size(THREADS, med_time, png_base + ".png")
    plot_time_by_size_err(THREADS, med_time, time_err, png_base + "_err.png")

    print("OK", csv_path)

if __name__ == "__main__":
    main()
