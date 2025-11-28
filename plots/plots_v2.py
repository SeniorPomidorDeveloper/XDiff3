# -*- coding: utf-8 -*-
import numpy as np
import matplotlib.pyplot as plt

def plot_time(threads, med_time_by_sim, out_png):
    x = np.array(threads, float)
    plt.figure(figsize=(9.5,6))
    for sim, med in med_time_by_sim.items():
        y = np.array([med[p] for p in threads], float)
        plt.plot(x, y, marker='o', label=f'sim={sim:.2f}')
    plt.xlabel("Число потоков"); plt.ylabel("Время, с")
    plt.title("Время vs число потоков")
    plt.grid(True); plt.legend(); plt.tight_layout(); plt.savefig(out_png, dpi=300); plt.close()

def plot_time_err(threads, med_time_by_sim, err_by_sim, out_png):
    """График времени с доверительными столбиками (σ)."""
    x = np.array(threads, float)
    plt.figure(figsize=(9.5,6))
    for sim in sorted(med_time_by_sim.keys()):
        med = med_time_by_sim[sim]
        err = err_by_sim[sim]
        y = np.array([med[p] for p in threads], float)
        yerr = np.array([err[p] for p in threads], float)
        plt.errorbar(x, y, yerr=yerr, marker='o', capsize=4, label=f'sim={sim:.2f}')
    plt.xlabel("Число потоков"); plt.ylabel("Время, с")
    plt.title("Время vs число потоков (с погрешностью)")
    plt.grid(True); plt.legend(); plt.tight_layout(); plt.savefig(out_png, dpi=300); plt.close()

def plot_speedup(threads, speedup_by_sim, out_png):
    x = np.array(threads, float)
    plt.figure(figsize=(9.5,6))
    for sim, S in speedup_by_sim.items():
        y = np.array([S[p] for p in threads], float)
        plt.plot(x, y, marker='o', label=f'sim={sim:.2f}')
    plt.plot(x, x, '--', label='идеальная')
    plt.xlabel("Число потоков"); plt.ylabel("Ускорение")
    plt.title("Ускорение vs число потоков")
    plt.grid(True); plt.legend(); plt.tight_layout(); plt.savefig(out_png, dpi=300); plt.close()

def plot_eff(threads, eff_by_sim, out_png):
    x = np.array(threads, float)
    plt.figure(figsize=(9.5,6))
    for sim, E in eff_by_sim.items():
        y = np.array([E[p] for p in threads], float)
        plt.plot(x, y, marker='o', label=f'sim={sim:.2f}')
    plt.xlabel("Число потоков"); plt.ylabel("Эффективность, %")
    plt.title("Эффективность vs число потоков")
    plt.grid(True); plt.legend(); plt.tight_layout(); plt.savefig(out_png, dpi=300); plt.close()

def plot_gb(threads, gb_theory, out_png):
    x = np.array(threads, float)
    plt.figure(figsize=(9.5,6))
    for sim, S in gb_theory.items():
        y = np.array([S[p] for p in threads], float)
        plt.plot(x, y, marker='o', label=f'GB sim={sim:.2f}')
    plt.plot(x, x, '--', label='идеальная')
    plt.xlabel("Число потоков"); plt.ylabel("Ускорение (GB)")
    plt.title("Густафсон—Барсис (без Амдаля)")
    plt.grid(True); plt.legend(); plt.tight_layout(); plt.savefig(out_png, dpi=300); plt.close()

# ---------- графики для разных размеров файла ----------

def _format_size(size_bytes: int) -> str:
    mb = size_bytes / (1024.0*1024.0)
    if mb >= 1.0:
        return f"{mb:.1f} MiB"
    kb = size_bytes / 1024.0
    return f"{kb:.0f} KiB"

def plot_time_by_size(threads, med_time_by_size, out_png):
    """Время vs потоки, разные размеры файла на одном графике."""
    x = np.array(threads, float)
    plt.figure(figsize=(9.5,6))
    for size, med in sorted(med_time_by_size.items()):
        y = np.array([med[p] for p in threads], float)
        label = _format_size(size)
        plt.plot(x, y, marker='o', label=label)
    plt.xlabel("Число потоков"); plt.ylabel("Время, с")
    plt.title("Время vs число потоков (разные размеры файла)")
    plt.grid(True); plt.legend(title="Размер файла"); plt.tight_layout(); plt.savefig(out_png, dpi=300); plt.close()

def plot_time_by_size_err(threads, med_time_by_size, err_by_size, out_png):
    """Время vs потоки, разные размеры файла, с погрешностями."""
    x = np.array(threads, float)
    plt.figure(figsize=(9.5,6))
    for size in sorted(med_time_by_size.keys()):
        med = med_time_by_size[size]
        err = err_by_size[size]
        y = np.array([med[p] for p in threads], float)
        yerr = np.array([err[p] for p in threads], float)
        label = _format_size(size)
        plt.errorbar(x, y, yerr=yerr, marker='o', capsize=4, label=label)
    plt.xlabel("Число потоков"); plt.ylabel("Время, с")
    plt.title("Время vs число потоков (разные размеры файла, с погрешностью)")
    plt.grid(True)
    plt.legend(title="Размер файла")
    plt.tight_layout()          # ← вот так
    plt.savefig(out_png, dpi=300)
    plt.close()


# ---------- график для разряженного (нулевого) массива ----------

def plot_time_sparse(threads, med_time, err_time, out_png,
                     title="Время vs число потоков (нулевой массив)"):
    """График времени с погрешностью для одного случая — нулевой (разряженный) файл."""
    x = np.array(threads, float)
    y = np.array([med_time[p] for p in threads], float)
    yerr = np.array([err_time[p] for p in threads], float)
    plt.figure(figsize=(9.5,6))
    plt.errorbar(x, y, yerr=yerr, marker='o', capsize=4, label="zeros")
    plt.xlabel("Число потоков"); plt.ylabel("Время, с")
    plt.title(title)
    plt.grid(True); plt.legend(); plt.tight_layout(); plt.savefig(out_png, dpi=300); plt.close()
