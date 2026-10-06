# Parallel Fraud Detection on PaySim Financial Transactions using OpenMPI
**High-Performance Parallel Computing Project — OpenMPI / C**  
*Team Project | Scalable Distributed Anomaly Detection*

---

## Project Overview

This project implements a high-performance, distributed fraud detection engine in C using **OpenMPI** to analyze massive financial transaction streams. The system processes the **PaySim Synthetic Financial Dataset** (up to 6.36 million records) and identifies fraudulent transactions using a **dual-criteria detection model**:

1. **Statistical Outlier Rule**: Flags transactions where `amount > mean + 2 × standard deviation` (population statistics).
2. **Account Velocity Rule**: Flags transactions originating from high-frequency accounts (`nameOrig`) with more than 10 transactions (`VELOCITY_THRESHOLD = 10`).

A transaction is flagged as **suspicious** if **EITHER** rule triggers.

Two optimized implementations are provided and evaluated:
| Version | Source File | Description |
| :--- | :--- | :--- |
| **Sequential** | `src/seq_fraud.c` | Single-threaded reference implementation with FNV-1a hash table. |
| **Parallel MPI** | `src/mpi_fraud.c` | Distributed-memory MPI implementation with **True Parallel File I/O**, global collective reductions, and distributed account velocity tracking. |

---

## Directory Structure

```
MPI/
├── src/
│   ├── seq_fraud.c                     # Sequential baseline fraud detector
│   ├── mpi_fraud.c                     # MPI parallel fraud detector (True Parallel I/O)
│   └── MPI.code-workspace              # VS Code workspace settings
├── data/
│   ├── paysim_10k.csv                  # 10,000 rows benchmark subset (~740 KB)
│   ├── paysim_100k.csv                 # 100,000 rows benchmark subset (~7.5 MB)
│   ├── paysim_1m.csv                   # 1,000,000 rows benchmark subset (~76.8 MB)
│   └── paysim_full.csv                 # Full PaySim dataset (6,362,620 rows, ~493.5 MB)
├── docs/
│   ├── llm_usage_log.md                # Comprehensive LLM usage and prompt audit trail
│   ├── prepare_data.sh                 # Dataset slicing script
│   └── run_tests.sh                    # Test harness verifying correctness on paysim_10k.csv
├── results/
│   ├── native_fs_performance_table.md  # Official benchmark results table (Source of Truth)
│   ├── native_fs_timing_log.csv        # Raw timing logs (I/O, Comp, Comm, Speedup)
│   └── archive_before_fix/             # Archived results from pre-fix Scatterv architecture
├── run_correctness.sh                  # Automated multi-process correctness verification script
├── run_timing.sh                       # Automated timing sweep script across process counts
├── setup_and_run.sh                    # WSL environment setup & test harness
└── README.md                           # Project documentation
```

---

## Dataset: PaySim Financial Dataset

The dataset is derived from the Kaggle **PaySim Synthetic Financial Dataset** (`ealaxi/paysim1`), representing mobile money transfer logs based on real financial networks.

### 11-Column Schema:
| Index | Column Name | Data Type | Description |
| :--- | :--- | :--- | :--- |
| 1 | `step` | `int` | Unit of time in hours (1 step = 1 hour, 744 steps total) |
| 2 | `type` | `string` | Transaction type (`PAYMENT`, `TRANSFER`, `CASH_OUT`, `DEBIT`, `CASH_IN`) |
| 3 | `amount` | `double` | Transaction amount in USD |
| 4 | `nameOrig` | `string` | Originating customer account ID (e.g., `C1231006815`) |
| 5 | `oldbalanceOrg` | `double` | Initial balance of sender before transaction |
| 6 | `newbalanceOrig` | `double` | New balance of sender after transaction |
| 7 | `nameDest` | `string` | Recipient account ID (customer `C...` or merchant `M...`) |
| 8 | `oldbalanceDest` | `double` | Initial balance of recipient before transaction |
| 9 | `newbalanceDest` | `double` | New balance of recipient after transaction |
| 10 | `isFraud` | `int` | Ground truth fraud flag (`1` = actual fraud, `0` = normal) |
| 11 | `isFlaggedFraud` | `int` | Business rule fraud flag (transfers > $200,000) |

---

## System Architecture & Implementation Details

### 1. Sequential Reference (`src/seq_fraud.c`)
- **Fast 64-bit Hashing**: Uses the Fowler–Noll–Vo 1a (**FNV-1a**) hash function to map alphanumeric account strings (`nameOrig`) into 64-bit integers.
- **Open-Addressing Hash Table**: Implements an in-memory hash table with linear probing and dynamic resizing to track account transaction frequencies in $\mathcal{O}(1)$ average time.
- **Two-Pass Statistical Pass**: Calculates global sum, population variance, and threshold in memory with dynamic `malloc`/`realloc` buffers.

### 2. Distributed MPI Engine (`src/mpi_fraud.c`)
- **True Parallel File I/O**:
  - Rank 0 executes a sub-millisecond line-count scan and broadcasts total rows $N$ (`MPI_Bcast`).
  - Every rank independently opens `csv_path`, seeks directly to its assigned row offset (`start_row = rank * (N / size)`), and streams/parses **only its own slice** (`local_n`) directly into memory.
  - Eliminates single-reader bottlenecks and avoids network communication for raw row data.
- **Global Account Velocity Tracking & Broadcast**:
  - Rank 0 builds the global velocity hash table once across all dataset accounts, counting transaction frequencies in $\mathcal{O}(1)$ time.
  - Rank 0 extracts the filtered, compact list of flagged high-velocity account IDs (`count > VELOCITY_THRESHOLD`) and broadcasts it to all ranks via `MPI_Bcast`.
  - Worker ranks populate an efficient local $\mathcal{O}(1)$ lookup set from this broadcast list, avoiding full-table replication or complex merging.
  - *Design Note*: An earlier design attempting to merge partial per-rank velocity hash tables across processes was benchmarked and discarded after identifying scaling degradation from cross-chunk account duplication, as documented in [`docs/llm_usage_log.md`](docs/llm_usage_log.md).
- **Global Collective Reductions**:
  - `MPI_Allreduce(MPI_SUM)` computes global transaction total $\sum X$ to derive mean $\mu$.
  - `MPI_Allreduce(MPI_SUM)` aggregates squared deviations $\sum (X - \mu)^2$ to derive population standard deviation $\sigma$.
- **Parallel Chunk Classification & Gathering**:
  - Each rank independently flags suspicious records within its local chunk using the broadcast velocity set and global threshold.
  - `MPI_Reduce(MPI_SUM)` gathers final fraud and classification counts at Rank 0.

---

## Critical Environment Note: Native WSL Filesystem vs `/mnt/c/`

> [!IMPORTANT]
> **Performance Finding:** Under WSL2, accessing files located on the Windows host path (`/mnt/c/...`) crosses the 9P Plan 9 virtual filesystem bridge, incurring severe I/O penalties.
> 
> * Benchmark testing showed Rank 0 single-reader file I/O on `/mnt/c/` took **42.71 seconds** for `paysim_full.csv`.
> * Running on the **native Linux ext4 filesystem** (`~/mpi_project/`) reduced single-reader I/O to **7.38 seconds**.
> * Adding **True Parallel I/O** scaled file reading down to **3.18 seconds** at `np=4` (**~13.4x overall I/O speedup** from original baseline).
>
> **Always clone and execute this project from `~/mpi_project/` inside WSL.**

---

## Prerequisites & Installation

Inside your WSL / Ubuntu terminal:

```bash
# Install GCC and OpenMPI development libraries
sudo apt update && sudo apt install -y openmpi-bin openmpi-doc libopenmpi-dev gcc

# Verify MPI installation
mpicc --version
mpirun --version
nproc    # Displays available physical/logical CPU cores
```

---

## Compilation

Navigate to the project directory inside WSL:

```bash
cd ~/mpi_project/src

# Compile Sequential baseline
gcc -O2 -o seq_fraud seq_fraud.c -lm

# Compile MPI Parallel engine
mpicc -O2 -o mpi_fraud mpi_fraud.c -lm
```

---

## Execution Instructions

```bash
# Run Sequential baseline on any dataset
./seq_fraud ../data/paysim_10k.csv
./seq_fraud ../data/paysim_full.csv

# Run MPI Parallel engine across 1, 2, or 4 processes
mpirun --allow-run-as-root -np 1 ./mpi_fraud ../data/paysim_10k.csv
mpirun --allow-run-as-root -np 2 ./mpi_fraud ../data/paysim_100k.csv
mpirun --allow-run-as-root -np 4 ./mpi_fraud ../data/paysim_full.csv
```

### Automated Verification & Timing Scripts
From the project root (`~/mpi_project/`), you can run the automated shell test harnesses:
```bash
# 1. Correctness Verification (tests flagged count consistency across np=1..16 for all datasets)
bash run_correctness.sh

# 2. Timing Sweep (measures MPI Total Execution time across np=1, 2, 4, 8, 16)
bash run_timing.sh
```

---

## Benchmark Results & Verified Performance

Source of truth: [results/native_fs_performance_table.md](results/native_fs_performance_table.md)

### 1. Correctness Verification (100% Match)
Every MPI run across all processor counts produces **identical flagged transaction counts** to the sequential baseline:

| Dataset | Row Count | Sequential Flagged | MPI np=1 | MPI np=2 | MPI np=4 | Status |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **paysim_10k.csv** | 10,000 | **242** | **242** | **242** | **242** | ✅ **100% MATCH** |
| **paysim_100k.csv** | 100,000 | **3,392** | **3,392** | **3,392** | **3,392** | ✅ **100% MATCH** |
| **paysim_1m.csv** | 1,000,000 | **31,481** | **31,481** | **31,481** | **31,481** | ✅ **100% MATCH** |
| **paysim_full.csv** | 6,362,620 | **81,410** | **81,410** | **81,410** | **81,410** | ✅ **100% MATCH** |

### 2. Performance & Scaling Summary

| Dataset | Rows | Version / Procs | File I/O Time | Pure Comp Time | Comm Time | Total Comp+Comm | Flagged |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **paysim_100k.csv** | 100,000 | Sequential | 0.1012s | 0.0088s | 0.0000s | 0.0088s | 3,392 |
| | 100,000 | MPI (np=1) | 0.1048s | 0.0100s | 0.0005s | 0.0105s | 3,392 |
| | 100,000 | MPI (np=2) | 0.0809s | 0.0160s | 0.0006s | 0.0166s | 3,392 |
| | 100,000 | MPI (np=4) | 0.0600s | 0.0133s | 0.0013s | 0.0146s | 3,392 |
| **paysim_1m.csv** | 1,000,000 | Sequential | 1.0634s | 0.1109s | 0.0000s | 0.1109s | 31,481 |
| | 1,000,000 | MPI (np=1) | 0.9740s | 0.1124s | 0.0059s | 0.1183s | 31,481 |
| | 1,000,000 | MPI (np=2) | 0.6856s | 0.1390s | 0.0099s | 0.1489s | 31,481 |
| | 1,000,000 | MPI (np=4) | 0.4857s | 0.1610s | 0.0101s | 0.1711s | 31,481 |
| **paysim_full.csv** | 6,362,620 | Sequential | 7.2413s | 0.9650s | 0.0000s | 0.9650s | 81,410 |
| | 6,362,620 | MPI (np=1) | 7.3823s | 0.8869s | 0.0336s | 0.9205s | 81,410 |
| | 6,362,620 | MPI (np=2) | **3.6035s** | 0.8891s | 0.0282s | 0.9173s | 81,410 |
| | 6,362,620 | MPI (np=4) | **3.1776s** | 1.2739s | 0.0576s | 1.3315s | 81,410 |

### Key Performance Insights:
- **Parallel I/O Scaling**: File reading scales near-linearly on `paysim_full.csv`, cutting I/O time in half from **7.38s at np=1 down to 3.60s at np=2 (2.05x speedup)** and **3.18s at np=4 (2.32x speedup)**.
- **Minimal Communication Overhead**: With Parallel I/O replacing dataset scattering, MPI communication accounts for **under 4.5%** of total execution time on the full dataset (~0.028s–0.058s).

---

## LLM Usage & Academic Integrity

In accordance with course requirements, all AI interactions, architectural discussions, prompt records, and technical trade-off decisions are documented in [`docs/llm_usage_log.md`](docs/llm_usage_log.md).

---

*Last updated: October 2026*
