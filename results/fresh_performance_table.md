# Fresh Benchmark Results — Native WSL Filesystem

| Dataset | Rows | Version / Procs | File I/O Time (s) | Pure Comp Time (s) | Comm Time (s) | Total Comp+Comm (s) | Total Exec Time (s) | Pure Comp Speedup | Total Speedup | Flagged | Expected | Correctness |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **paysim_10k.csv** | 10,000 | Sequential (1 core) | 0.026117s | 0.002370s | 0.000000s | 0.002370s | 0.028487s | 1.00x | 1.00x | **242** | 242 | ✅ PASS |
| **paysim_10k.csv** | 10,000 | MPI Parallel I/O (np=1) | 0.017962s | 0.001526s | 0.000076s | 0.001602s | 0.019564s | 1.55x | 1.46x | **242** | 242 | ✅ PASS |
| **paysim_10k.csv** | 10,000 | MPI Parallel I/O (np=2) | 0.022435s | 0.003025s | 0.000370s | 0.003395s | 0.025830s | 0.78x | 1.10x | **242** | 242 | ✅ PASS |
| **paysim_10k.csv** | 10,000 | MPI Parallel I/O (np=4) | 0.010810s | 0.005759s | 0.000488s | 0.006247s | 0.017057s | 0.41x | 1.67x | **242** | 242 | ✅ PASS |
| **paysim_100k.csv** | 100,000 | Sequential (1 core) | 0.262814s | 0.037314s | 0.000000s | 0.037314s | 0.300128s | 1.00x | 1.00x | **3,392** | 3,392 | ✅ PASS |
| **paysim_100k.csv** | 100,000 | MPI Parallel I/O (np=1) | 0.247582s | 0.039210s | 0.001546s | 0.040756s | 0.288338s | 0.95x | 1.04x | **3,392** | 3,392 | ✅ PASS |
| **paysim_100k.csv** | 100,000 | MPI Parallel I/O (np=2) | 0.136239s | 0.028448s | 0.001147s | 0.029596s | 0.165835s | 1.31x | 1.81x | **3,392** | 3,392 | ✅ PASS |
| **paysim_100k.csv** | 100,000 | MPI Parallel I/O (np=4) | 0.170425s | 0.077332s | 0.009959s | 0.087292s | 0.257717s | 0.48x | 1.16x | **3,392** | 3,392 | ✅ PASS |
| **paysim_1m.csv** | 1,000,000 | Sequential (1 core) | 2.148343s | 0.312718s | 0.000000s | 0.312718s | 2.461061s | 1.00x | 1.00x | **31,481** | 31,481 | ✅ PASS |
| **paysim_1m.csv** | 1,000,000 | MPI Parallel I/O (np=1) | 1.511391s | 0.178514s | 0.011136s | 0.189650s | 1.701041s | 1.75x | 1.45x | **31,481** | 31,481 | ✅ PASS |
| **paysim_1m.csv** | 1,000,000 | MPI Parallel I/O (np=2) | 0.833796s | 0.175666s | 0.013299s | 0.188965s | 1.022761s | 1.78x | 2.41x | **31,481** | 31,481 | ✅ PASS |
| **paysim_1m.csv** | 1,000,000 | MPI Parallel I/O (np=4) | 0.685118s | 0.327963s | 0.018825s | 0.346788s | 1.031906s | 0.95x | 2.38x | **31,481** | 31,481 | ✅ PASS |
| **paysim_full.csv** | 6,362,620 | Sequential (1 core) | 8.353176s | 1.947315s | 0.000000s | 1.947315s | 10.300491s | 1.00x | 1.00x | **81,410** | 81,410 | ✅ PASS |
| **paysim_full.csv** | 6,362,620 | MPI Parallel I/O (np=1) | 9.589183s | 1.578806s | 0.162365s | 1.741170s | 11.330353s | 1.23x | 0.91x | **81,410** | 81,410 | ✅ PASS |
| **paysim_full.csv** | 6,362,620 | MPI Parallel I/O (np=2) | 5.543247s | 1.452441s | 0.101901s | 1.554342s | 7.097589s | 1.34x | 1.45x | **81,410** | 81,410 | ✅ PASS |
| **paysim_full.csv** | 6,362,620 | MPI Parallel I/O (np=4) | 4.264933s | 1.608725s | 0.105693s | 1.714418s | 5.979351s | 1.21x | 1.72x | **81,410** | 81,410 | ✅ PASS |
