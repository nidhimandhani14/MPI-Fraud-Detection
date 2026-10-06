# Archived Results: Pre-Fix MPI Scatterv Design

This directory contains experimental benchmark results and logs from our initial MPI implementation prior to resolving the I/O bottleneck.

### Architecture Context
In the original design:
- Rank 0 performed single-threaded serial CSV parsing of the entire dataset.
- Rank 0 then distributed data chunks to worker ranks via `MPI_Scatterv` and gathered results via `MPI_Reduce` / `MPI_Gatherv`.

### Bottleneck Analysis
While pure computation scaled across cores, single-rank I/O and large memory scatter overhead created an Amdahl's Law bottleneck, causing total execution time on large datasets (e.g., `paysim_full.csv`) to lag behind the sequential baseline.

These results are preserved as the "before" state documented in our project report's bottleneck analysis.

### Files
- `scatterv_performance_table.md` / `scatterv_summary_table.md`: Detailed performance metrics from the Scatterv experiments.
- `scatterv_timing_log.csv`: Raw timing records from the Scatterv sweep across process counts (`np=1, 2, 4, 8, 16`).
- `performance_summary_table.md`: Earlier consolidated summary table from the Scatterv runs.

> **Note:** For our current, optimized implementation (Parallel Independent File I/O with byte-offset domain decomposition), refer to [`results/native_fs_performance_table.md`](../native_fs_performance_table.md) and [`results/native_fs_timing_log.csv`](../native_fs_timing_log.csv).
