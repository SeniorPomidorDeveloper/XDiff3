# -*- coding: utf-8 -*-
import ctypes as C

def load_lib(path: str):
    lib = C.CDLL(path)
    lib.xdiff3_diff_time_from_buffers.argtypes = [C.POINTER(C.c_uint8), C.c_size_t,
                                                  C.POINTER(C.c_uint8), C.c_size_t,
                                                  C.c_int]
    lib.xdiff3_diff_time_from_buffers.restype  = C.c_double
    return lib

def run_from_buffers(lib, src: bytes, tgt: bytes, threads: int) -> float:
    src_arr = (C.c_uint8 * len(src)).from_buffer_copy(src)
    tgt_arr = (C.c_uint8 * len(tgt)).from_buffer_copy(tgt)
    return float(lib.xdiff3_diff_time_from_buffers(src_arr, len(src), tgt_arr, len(tgt), int(threads)))