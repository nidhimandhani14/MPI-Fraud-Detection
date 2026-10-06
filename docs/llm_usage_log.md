# LLM Usage Log
**Course:** Parallel Programming (OpenMP/MPI/CUDA)  
**Project:** Fraud Detection using MPI (PaySim Dataset)  
**Date:** October 2026  
**Team size:** 4

---

## Session 1 — Prototype Exploration, Synthetic 30-Row Validation & ULB Dataset Evaluation

### Objectives & Exploratory Work
1. **Initial MPI Mechanics Validation**:
   - Built an initial proof-of-concept in C using OpenMPI to establish basic parallel collective patterns (`MPI_Bcast`, `MPI_Allreduce`, `MPI_Reduce`).
   - Validated basic statistical anomaly detection (flagging transactions with amount $> \mu + 2\sigma$) on a hand-crafted 30-row synthetic `transactions.csv` to ensure MPI mechanics functioned properly under WSL/Ubuntu.
2. **Dataset Evaluation & Migration Decision**:
   - Evaluated the Kaggle ULB Credit Card Fraud dataset (284,807 transactions with anonymized PCA features `V1`..`V28`, `Time`, `Amount`, `Class`).
   - **Rationale for Moving to PaySim**:
     - *Lack of Relational Entity Attributes*: The ULB dataset's PCA features obscured entity identifiers, preventing realistic relational graph or account-velocity rules.
     - *Need for Scalable High-Performance Workload*: The PaySim synthetic financial dataset (`paysim_full.csv`, 6,362,620 rows, ~493.5 MB) provided named customer entities (`nameOrig`, `nameDest`), timestamps, transaction types, and sufficient data volume to realistically benchmark distributed hash tables, memory scaling, and parallel I/O throughput.

---

## Session 2 — PaySim Migration, MPI Account Velocity Logic & Performance Sweep

### Requirements Implemented
1. Removed old 30-row `transactions.csv` and ULB credit card code references.
2. Migrated dataset to PaySim synthetic financial dataset (`paysim_full.csv`, 6,362,620 rows, 493.5 MB).
3. Created 3 slice datasets for staged performance testing:
   - `paysim_10k.csv` (10,000 data rows)
   - `paysim_100k.csv` (100,000 data rows)
   - `paysim_1m.csv` (1,000,000 data rows)
4. Updated CSV parser in `seq_fraud.c` and `mpi_fraud.c` for 11 PaySim columns:
   `step,type,amount,nameOrig,oldbalanceOrg,newbalanceOrig,nameDest,oldbalanceDest,newbalanceDest,isFraud,isFlaggedFraud`
5. Extended detection rules:
   - **Rule 1 (Amount Rule)**: Flag if `amount > mean + 2*stddev` (computed globally via `MPI_Allreduce`).
   - **Rule 2 (Account Velocity Rule)**: Flag if originator account `nameOrig` total transaction count > `VELOCITY_THRESHOLD` (10 transactions).
   - **Combined Flag**: Flagged as suspicious if EITHER rule triggers.
6. Handled dynamic memory allocation (`malloc`/`realloc`) to support scaling up to 6.36 million rows without stack/buffer overflow.
7. Conducted correctness verification and full multi-core performance sweep (Sequential, np=1, 2, 4, 8, 16).

---

## Technical Challenges & Solutions (Session 2)

### Challenge 1: Parallel Account Velocity Counting with Sparse 64-bit Account IDs
- **Problem**: `nameOrig` strings (e.g. `C1231006815`) are sparse and non-numeric, making direct array indexing impossible.
- **Solution**:
  - Implemented FNV-1a 64-bit hashing (`hash_string`) to convert account strings to `uint64_t account_id`.
  - Built a fast, open-addressing Hash Table (`HashTable` with linear probing) in C to count account frequencies in $\mathcal{O}(1)$ time per transaction.

### Challenge 2: Merging Partial Account Counts Across MPI Ranks
- **Problem**: In MPI, each rank processes a local chunk `[local_start, local_end)` and only sees local account counts. A naive merge would require $\mathcal{O}(N)$ communication or global hash synchronization.
- **Solution**:
  - Each rank builds a local hash table for accounts in its chunk.
  - Defined a custom C struct `AccountEntry { uint64_t account_id; int count; }` and registered a custom MPI Datatype (`MPI_Type_create_struct`).
  - Used `MPI_Gather` to collect entry counts, then `MPI_Gatherv` to collect non-zero local pairs at Rank 0.
  - Rank 0 merges the gathered pairs into a global hash table, evaluates `is_high_velocity` boolean flags for all rows, and broadcasts the flag array to all processes using `MPI_Bcast`.

### Challenge 3: Correctness Guarantee Across Varying Process Counts
- **Problem**: Splitting rows unevenly or non-deterministic reduction could introduce discrepancies between Sequential and MPI runs.
- **Solution**:
  - Used `MPI_Allreduce` for global sum of amounts and global sum of squared deviations.
  - Used Rank 0 global hash table merge for account counts before broadcasting velocity flags.
  - **Result**: Sequential and all MPI runs (np=1, 2, 4, 8, 16) produce **100% bit-identical suspicious transaction counts** across all dataset sizes.

---

## Session 3 — Architecture Optimization, Scaling Bug Fixes, Native Filesystem Migration & True Parallel I/O

### Summary of Prompt Interactions & Critical Modifications (Verbatim Prompts)

| Sr. No. | Purpose | Prompt Used (Exact Verbatim Prompt) | How Response Was Used | Modification / Rejection & Verification |
| :--- | :--- | :--- | :--- | :--- |
| **1** | **Dataset Distribution (Scatterv vs Bcast)** | *"Our current mpi_fraud.c uses MPI_Bcast to send the ENTIRE dataset to every process, which is why MPI is slower than sequential on large files (confirmed: ~45s MPI vs ~16.6s sequential on the full 6.36M-row dataset, with MPI_Bcast alone costing 15-20s to copy 178MB to each process).<br><br>Please rewrite the data distribution to use MPI_Scatterv instead of MPI_Bcast:<br>- Rank 0 reads the file and computes how many rows go to each rank<br>- Use MPI_Scatterv to send each rank ONLY its own chunk of rows (not the whole dataset)<br>- Keep MPI_Allreduce for combining global mean/stddev as before (this only sends small numbers, not the full dataset, so it's cheap)<br>- Keep MPI_Gatherv to collect flagged results back to rank 0<br><br>Then re-run the same benchmark (all 4 dataset sizes, np=1/2/4/8/16) and show me the new timing table. I expect MPI to now be clearly FASTER than sequential on the large datasets, since we're no longer copying the entire dataset to every process.<br><br>Also keep the same correctness check as before — flagged counts must still match sequential exactly."* | Replaced full-array `MPI_Bcast` with `MPI_Scatterv` so each rank allocates and receives strictly $N/P$ rows. | **Root cause:** Initial LLM architecture broadcasted the entire 500 MB dataset to all ranks ($P \times 500\text{ MB} = 8\text{ GB}$ for $P=16$), exhausting RAM bandwidth.<br>**Verification:** Memory footprint reduced by factor of $P$; correctness verified 100% matching on all datasets. |
| **2** | **Fixing Velocity Hash Table Scaling Bug** | *"We found the root cause of mpi_fraud.c performing worse with more processes: the account-velocity rule is parallelized incorrectly. Each rank builds a separate local hash table of accounts in its chunk, then Gather/Gatherv reassembles and merges them on rank 0 — but since data is split by row position (not grouped by account), the same account ends up duplicated across multiple ranks' local tables, so the amount of data to gather and merge GROWS as process count increases, instead of staying constant.<br><br>Fix: since rank 0 already has the full dataset in memory right after reading the CSV (before scattering), build the velocity hash table ONCE, directly on rank 0, from the full data — exactly like seq_fraud.c does. Remove the local_ht per-rank building, the MPI_Gather/MPI_Gatherv/MPI_ACCOUNT_ENTRY struct machinery, and the rank-0 merge-from-gathered-pairs block entirely. Keep the existing broadcast of the (small) high-velocity account list to all ranks, and keep each rank's local lookup check against that broadcast list — that part was already correct.<br><br>Keep the amount/mean/stddev Scatterv + Allreduce logic completely unchanged — that part scales correctly and isn't the problem.<br><br>After making this change:<br>1. Recompile mpi_fraud.c<br>2. Re-run correctness check: seq_fraud vs mpi_fraud at np=1,2,4 only on all 4 dataset sizes<br>3. Confirm flagged counts still match exactly<br>4. Show the new timing table and confirm computation/communication time no longer grows with process count — it should now stay roughly flat or decrease as np increases from 1 to 4"* | Identified hash-duplication bottleneck in per-rank local hash tables + Gatherv merge. Refactored to build the hash table once on Rank 0 directly from loaded/gathered IDs, then broadcast only the compact high-velocity list. | **Critical Modification of LLM Design:** The initial LLM suggestion to build $P$ separate local hash tables, package them into custom structs, `MPI_Gatherv` them, and re-hash them at Rank 0 caused massive redundant hashing and cache thrashing.<br>**Verification:** Pure computation time dropped from 2.44s at np=1 to 1.40s at np=2 on full dataset. |
| **3** | **Eliminating WSL 9P Filesystem Bottleneck & Native Ext4 Benchmark** | *"The native WSL filesystem test worked — confirmed major I/O speedup (sequential I/O dropped from 13.3s to 6.3s, MPI np=4 I/O dropped from 42.7s to 8.8s on the full dataset, when run from ~/mpi_project instead of /mnt/c/Users/ESHITA/OneDrive/Desktop/MPI).<br><br>Please now run the complete benchmark sweep from ~/mpi_project (the native WSL copy, NOT the /mnt/c/ OneDrive path):<br><br>1. Run seq_fraud and mpi_fraud at np=1, np=2, np=4 across all 4 datasets: paysim_10k.csv, paysim_100k.csv, paysim_1m.csv, paysim_full.csv<br>2. For each run, record: I/O time, pure computation time, MPI communication time, and total time<br>3. Confirm correctness (flagged transaction counts must match across all configurations)<br>4. Generate a clean Markdown table comparing Sequential vs np=1, np=2, np=4 for all 4 datasets<br>5. Save raw timing records to results/native_fs_timing_log.csv and the formatted table to results/native_fs_performance_table.md"* | Migrated project and data from `/mnt/c/...` (Windows NTFS via WSL 9P bridge) to native Linux ext4 path (`~/mpi_project/`). | **Finding:** 9P Plan 9 virtual filesystem bridge added ~35s of artificial disk latency. Moving to native WSL filesystem reduced single-reader I/O from 42.7s to 7.87s (5.5x speedup), but I/O still did not scale with process count. |
| **4** | **True Parallel File I/O Implementation** | *"We've confirmed the native WSL filesystem fix improved raw I/O speed, but File I/O time still does NOT scale down with process count (full dataset: np=1 7.87s, np=2 8.47s, np=4 10.32s — flat/increasing, not decreasing). This is because only rank 0 reads the file and broadcasts/scatters data — file reading itself was never parallelized.<br><br>Fix this now: change mpi_fraud.c so EVERY rank reads the file directly and independently, each parsing only its own assigned chunk of rows:<br>- Rank 0 does a fast first pass (e.g. counting lines or using file size) to determine total row count N, and broadcasts N to all ranks<br>- Each rank computes its start row and row count: start = rank * (N/size), count = N/size (+ remainder for last rank)<br>- Each rank opens the CSV file, seeks/scans to its start row, and parses ONLY its assigned count rows directly into its local arrays — eliminating the need for rank 0 to load the full dataset or scatter row data<br>- Keep the statistical reductions (MPI_Allreduce for sum and sq_diff) and account velocity broadcast as they are<br><br>After implementing, recompile and benchmark on paysim_full.csv at np=1, 2, 4 from ~/mpi_project. File I/O time should now scale down roughly proportional to 1/np as process count increases. Verify correctness still matches sequential exactly."* | Replaced serial Rank 0 reader with independent parallel file streams: Rank 0 does a fast 1MB-buffered newline scan (~0.04s) and broadcasts $N$; all ranks seek to `start_row` and parse `local_n` rows simultaneously. | **Verification:** On `paysim_full.csv`, File I/O dropped from 7.38s at np=1 to **3.60s at np=2 (2.05x linear speedup)** and **3.18s at np=4 (2.32x speedup)**. Overall I/O overhead reduced by **92.6% (13.4x faster)** compared to the original `/mnt/c/` baseline. |

---

## Detailed Case Studies of Modified / Rejected LLM Output

### Case Study 1: The Memory Replication Flaw in `MPI_Bcast` Data Distribution
* **Initially Suggested / Implemented:** Rank 0 loaded all 6.36M rows and used four consecutive `MPI_Bcast` calls (`global_steps`, `global_amounts`, `global_account_ids`, `global_is_frauds`) to copy the entire dataset to every rank, with each rank looping over `[rank * (N/size), (rank + 1) * (N/size))`.
* **Why it seemed reasonable:** It simplified code logic; every rank had the complete dataset in memory, avoiding dynamic array slicing or displacement arithmetic.
* **Flaw Detected during Benchmarking:** At scale (6.36M rows), broadcasting ~500 MB of data to 16 processes required allocating ~8 GB across processes, thrashing cache hierarchies and memory buses. Total communication time exceeded 2.5 seconds, making MPI slower than sequential execution.
* **Fix & Verification:** Rewrote distribution using `MPI_Scatterv`. Each process allocates memory strictly for `local_n = N/size` elements. Correctness was verified identically across all 4 dataset sizes, and memory usage scaled down inversely with process count.

---

### Case Study 2: The Account-Velocity Hash Table Duplication & Merge Overhead
* **Initially Suggested / Implemented:** Each MPI rank independently counted transaction frequencies for its assigned chunk in a local hash table. Ranks extracted non-zero `(account_id, count)` pairs into a custom `AccountEntry` struct, transmitted them via `MPI_Gatherv` to Rank 0, and Rank 0 merged them into a global hash table.
* **Why it seemed reasonable:** It appeared to distribute the hashing workload across all available CPU cores.
* **Flaw Detected during Benchmarking:** Profiling pure computation time across process counts revealed a severe scaling bug:
  * Full dataset (`paysim_full.csv`):
    * Sequential Pure Comp: **1.00s**
    * MPI np=1 Pure Comp: **1.16s**
    * MPI np=8 Pure Comp: **1.58s**
    * MPI np=16 Pure Comp: **2.19s** *(over 2x slower than sequential!)*
  * **Root Cause:** Because transactions for any given account are randomly distributed across the dataset, virtually every active account was inserted into all $P$ local hash tables. Extracting $P$ hash tables, copying thousands of structs into dynamic arrays, gathering them, and re-inserting them into a global hash table at Rank 0 performed far *more* total hash insertions, memory allocations, and linear-probing collisions than a single sequential pass.
* **Fix & Verification:** Modified Rank 0 to build the velocity hash table once directly from the gathered/loaded account ID array ($\mathcal{O}(N)$ single pass), extract the compact high-velocity list ($K$ accounts where $K \ll N$), and broadcast only that small list ($K \times 8$ bytes) via `MPI_Bcast`. Each rank then performs fast $\mathcal{O}(1)$ lookups on its local chunk. This immediately restored parallel computation scaling (np=1: 2.44s $\rightarrow$ np=2: 1.40s).

---

### Case Study 3: Single-Reader I/O Bottleneck and True Parallel File I/O
* **Initially Suggested / Implemented:** Only Rank 0 opened and parsed the CSV file (`load_csv_rank0`), while other ranks waited idle at `MPI_Barrier`.
* **Why it seemed reasonable:** Standard pattern for introductory MPI programs; avoided multi-process file contention and file-seek coordination.
* **Flaw Detected during Benchmarking:**
  1. Under WSL2 with Windows host mounting (`/mnt/c/...`), single-reader I/O took **42.71 seconds** on `paysim_full.csv`.
  2. Moving to native WSL filesystem (`~/mpi_project/`) reduced single-reader I/O to **7.38–7.87 seconds**, but the I/O time remained flat/increasing regardless of process count (`np=1`: 7.87s, `np=2`: 8.47s, `np=4`: 10.32s), because file parsing was entirely serial.
* **Fix & Verification:** Implemented **True Parallel File I/O**:
  1. Rank 0 runs a rapid line-counter scanning newlines in 1MB chunks (~0.04s) and broadcasts total rows $N$.
  2. All ranks compute `start_row = rank * (N/size)` and `local_n = N/size`.
  3. All ranks open the CSV file in parallel, skip to their starting line, and stream/parse **only their assigned slice of rows** directly into local memory.
  4. **Benchmark Verification ([results/native_fs_performance_table.md](../results/native_fs_performance_table.md)):**
     * `paysim_full.csv` I/O time dropped from **7.38s (np=1)** to **3.60s (np=2, 2.05x speedup)** and **3.18s (np=4, 2.32x speedup)**.
     * Flagged transaction counts remained **100% identical** across all datasets.
     * Total file reading overhead was reduced by **92.6%** compared to the initial `/mnt/c/` benchmark.

---

## Final Verified Correctness & Benchmark Summary

All runs executed on the **native WSL ext4 filesystem** (`~/mpi_project/`) with full dataset verification:

| Dataset | Rows | Sequential Flagged | MPI np=1 | MPI np=2 | MPI np=4 | Status |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| `paysim_10k.csv` | 10,000 | **242** | **242** | **242** | **242** | ✅ **100% MATCH** |
| `paysim_100k.csv` | 100,000 | **3,392** | **3,392** | **3,392** | **3,392** | ✅ **100% MATCH** |
| `paysim_1m.csv` | 1,000,000 | **31,481** | **31,481** | **31,481** | **31,481** | ✅ **100% MATCH** |
| `paysim_full.csv` | 6,362,620 | **81,410** | **81,410** | **81,410** | **81,410** | ✅ **100% MATCH** |

*Last updated: October 2026*
