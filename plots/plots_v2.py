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