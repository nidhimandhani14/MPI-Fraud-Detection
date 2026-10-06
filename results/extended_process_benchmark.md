# Extended Process Benchmark Results (np=8)

> **Important**: This file contains supplementary benchmark results for testing **np=8** on the existing, unmodified MPI codebase. All existing verified benchmark files (`native_fs_performance_table.md`, `native_fs_timing_log.csv`, `fresh_performance_table.md`, `fresh_timing_log.csv`) and project source files remain completely untouched.

---

## 1. Pre-Run Verification (np=4 Baseline)

Prior to executing the `np=8` benchmark, the existing `np=4` configuration was executed with the existing binary and datasets to verify environment stability and correctness.

| Dataset | Total Rows | Target Process Count | Flagged Transactions | Expected | Correctness Status |
| :--- | :--- | :--- | :--- | :--- | :--- |
| **paysim_1m.csv** | 1,000,000 | MPI `np=4` | **31,481** | 31,481 | ✅ PASS |
| **paysim_full.csv** | 6,362,620 | MPI `np=4` | **81,410** | 81,410 | ✅ PASS |

---

## 2. Extended Benchmark Results (np=8)

Both datasets were executed using the exact same MPI executable (`mpi_fraud`) and timing instrumentation:
- **Command**: `mpirun --oversubscribe -np 8 ./mpi_fraud <dataset_path>`
- **Timing Breakdown Recorded**: File I/O Time, Pure Computation Time, MPI Communication Time, Total Comp+Comm Time, and Total Execution Time (`File I/O + Total Comp+Comm`).

| Dataset | Rows | Version / Procs | File I/O Time (s) | Pure Comp Time (s) | Comm Time (s) | Total Comp+Comm (s) | Total Exec Time (s) | Flagged | Expected | Correctness |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **paysim_1m.csv** | 1,000,000 | MPI Parallel I/O (np=8) | 0.618470s | 0.498817s | 0.044842s | 0.543659s | 1.162129s | **31,481** | 31,481 | ✅ PASS |
| **paysim_full.csv** | 6,362,620 | MPI Parallel I/O (np=8) | 4.880647s | 3.057386s | 0.238880s | 3.296266s | 8.176913s | **81,410** | 81,410 | ✅ PASS |

---

## 3. Side-by-Side Comparison: np=4 vs. np=8

Comparison of the verified baseline (`np=4` from `native_fs_performance_table.md`) against the newly tested `np=8` configuration.

### A. 1M Dataset (`paysim_1m.csv` — 1,000,000 Rows)

| Metric | Verified np=4 | New np=8 | Delta / Observation |
| :--- | :--- | :--- | :--- |
| **Flagged Transactions** | **31,481** | **31,481** | **Exact match (100% Correctness)** |
| **File I/O Time** | 0.485683s | 0.618470s | +0.132787s (+27.3%) |
| **Pure Computation Time** | 0.160981s | 0.498817s | +0.337836s (+209.9%) |
| **MPI Communication Time** | 0.010135s | 0.044842s | +0.034707s (+342.5%) |
| **Total Comp + Comm Time** | 0.171116s | 0.543659s | +0.372543s (+217.7%) |
| **Total Execution Time** | 0.656799s | 1.162129s | +0.505330s (+76.9%) |
| **Pure Comp Speedup (vs Seq)** | 0.69x | 0.22x | Lower computational efficiency |

### B. Full Dataset (`paysim_full.csv` — 6,362,620 Rows)

| Metric | Verified np=4 | New np=8 | Delta / Observation |
| :--- | :--- | :--- | :--- |
| **Flagged Transactions** | **81,410** | **81,410** | **Exact match (100% Correctness)** |
| **File I/O Time** | 3.177566s | 4.880647s | +1.703081s (+53.6%) |
| **Pure Computation Time** | 1.273878s | 3.057386s | +1.783508s (+140.0%) |
| **MPI Communication Time** | 0.057637s | 0.238880s | +0.181243s (+314.5%) |
| **Total Comp + Comm Time** | 1.331515s | 3.296266s | +1.964751s (+147.6%) |
| **Total Execution Time** | 4.509081s | 8.176913s | +3.667832s (+81.3%) |
| **Pure Comp Speedup (vs Seq)** | 0.76x | 0.32x | Lower computational efficiency |

---

## 4. Key Performance Insights

1. **Strict Correctness Maintained**:
   - Both datasets produce **100% exact fraud detection results** (`31,481` for 1M and `81,410` for Full).
2. **Parallel Scaling Overhead**:
   - As process count increases from 4 to 8 on this local host (Intel Core i5-1235U with 2 Performance cores and 8 Efficient cores), process scheduling, context switching, and collective MPI reductions (`MPI_Allreduce`, `MPI_Reduce`) introduce overhead that outweighs the computational workload per core.
3. **Optimal Operating Point**:
   - `np=4` remains the optimal configuration for this hardware architecture, achieving the lowest total execution time (0.657s on 1M and 4.509s on Full) and highest parallel efficiency.
