# -*- coding: utf-8 -*-
import os, csv, glob
from datetime import datetime
from core_ctypes import load_lib, run_from_buffers
from metrics import aggregate, gustafson_barsis
from plots_v2 import plot_time, plot_speedup, plot_eff, plot_gb

def read_file(path: str) -> bytes:
    with open(path, "rb") as f:
        return f.read()

def discover_pairs(data_dir):
    # ищем пары simXX_source.bin / simXX_target.bin
    pairs = {}
    for src_path in sorted(glob.glob(os.path.join(data_dir, "sim*_source.bin"))):
        tag = os.path.basename(src_path).split("_source.bin")[0]  # sim20
        tgt_path = os.path.join(data_dir, f"{tag}_target.bin")
        if os.path.exists(tgt_path):
            # similarity из имени: sim20 -> 0.20
            sim_num = int(tag.replace("sim",""))
            sim = sim_num/100.0
            pairs[sim] = (src_path, tgt_path)
    return pairs

def env_list(name, cast, default):
    txt = os.getenv(name, "")
    if not txt: return default
    return [cast(x) for x in txt.replace(",", " ").split() if x.strip()]

def main():
<<<<<<< HEAD
    LIB_PATH = os.getenv("LIB_PATH", "../build/libxdiff3_bench.so")
=======
    LIB_PATH = os.getenv("LIB_PATH", "../build/libxdiff3.so")
>>>>>>> 4b06621800aef0ce4747503dcd605fe73896efc6
    DATA_DIR = os.getenv("DATA_DIR", "./data")
    THREADS  = env_list("THREADS_LIST", int, list(range(1, 41)))
    ATTEMPTS = int(os.getenv("ATTEMPTS", "5"))
    OUT_DIR  = os.getenv("OUT_DIR", ".")

    pairs = discover_pairs(DATA_DIR)
    if not pairs:
        raise SystemExit(f"Не найдены пары файлов в {DATA_DIR}")

    lib = load_lib(LIB_PATH)
    os.makedirs(OUT_DIR, exist_ok=True)

    times = {s: {p: [] for p in THREADS} for s in sorted(pairs.keys())}

    # загружаем файлы в память (чтобы исключить I/O из замера)
    buffers = {s: (read_file(pairs[s][0]), read_file(pairs[s][1])) for s in pairs}

    for s in sorted(pairs.keys()):
        src, tgt = buffers[s]
        for p in THREADS:
            for _ in range(ATTEMPTS):
                dt = run_from_buffers(lib, src, tgt, p)
                times[s][p].append(dt)

    # агрегаты
    med_time = {}
    speedup  = {}
    eff      = {}
    for s in sorted(times.keys()):
        med, S, E = aggregate(times[s])
        med_time[s] = med
        speedup[s]  = S
        eff[s]      = E

    # теория GB
    gb = gustafson_barsis(THREADS, sorted(times.keys()))

    ts = datetime.now().strftime("%Y%m%d_%H%M%S")
    csv_path = os.path.join(OUT_DIR, f"xdiff3_results_{ts}.csv")
    with open(csv_path, "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["similarity","threads","time_med_s","speedup","efficiency_percent"])
        for s in sorted(times.keys()):
            for p in THREADS:
                w.writerow([s, p, f"{med_time[s][p]:.9f}", f"{speedup[s][p]:.6f}", f"{eff[s][p]:.3f}"])

    plot_time(THREADS, med_time, os.path.join(OUT_DIR, f"time_{ts}.png"))
    plot_speedup(THREADS, speedup, os.path.join(OUT_DIR, f"speedup_{ts}.png"))
    plot_eff(THREADS, eff, os.path.join(OUT_DIR, f"eff_{ts}.png"))
    plot_gb(THREADS, gb, os.path.join(OUT_DIR, f"gb_{ts}.png"))

    print("OK", csv_path)

if __name__ == "__main__":
    main()