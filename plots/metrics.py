# -*- coding: utf-8 -*-
import numpy as np

def aggregate(times_dict):
    th = sorted(times_dict.keys())
    med_t = {p: float(np.median(times_dict[p])) for p in th}
    t1 = med_t[1]
    S  = {p: t1/med_t[p] for p in th}
    E  = {p: (S[p]/p)*100.0 for p in th}
    return med_t, S, E

def gustafson_barsis(threads, similarities):
    out = {}
    for s in similarities:
        a = 1.0 - s
        out[s] = {p: (p - a*(p-1)) for p in threads}
    return out