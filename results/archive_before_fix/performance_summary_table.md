# Performance & Scaling Results Table

| Dataset | Dataset Rows | Version / Process Count | File I/O Time (s) | Pure Computation Time (s) | Communication Time (s) | Total Execution Time (s) | Suspicious Flagged | Correctness |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **paysim_10k.csv** | 10000 | Sequential (1 core) | 0.021043s | secondss | 0.000000s | secondss | **242** | ✅ PASS |
| **paysim_10k.csv** | 10000 | MPI (np=1) | 0.201566s | secondss | (7.2%)s | secondss | **242** | ✅ PASS |
| **paysim_10k.csv** | 10000 | MPI (np=2) | 0.177145s | secondss | (40.2%)s | secondss | **242** | ✅ PASS |
| **paysim_10k.csv** | 10000 | MPI (np=4) | 0.113221s | secondss | (44.8%)s | secondss | **242** | ✅ PASS |
| **paysim_10k.csv** | 10000 | MPI (np=8) | 0.112756s | secondss | (63.7%)s | secondss | **242** | ✅ PASS |
| **paysim_10k.csv** | 10000 | MPI (np=16) | 0.572333s | secondss | (77.6%)s | secondss | **242** | ✅ PASS |
| **paysim_100k.csv** | 100000 | Sequential (1 core) | 0.292777s | secondss | 0.000000s | secondss | **3392** | ✅ PASS |
| **paysim_100k.csv** | 100000 | MPI (np=1) | 1.047286s | secondss | (6.0%)s | secondss | **3392** | ✅ PASS |
| **paysim_100k.csv** | 100000 | MPI (np=2) | 0.929619s | secondss | (18.4%)s | secondss | **3392** | ✅ PASS |
| **paysim_100k.csv** | 100000 | MPI (np=4) | 0.816835s | secondss | (40.5%)s | secondss | **3392** | ✅ PASS |
| **paysim_100k.csv** | 100000 | MPI (np=8) | 0.805837s | secondss | (32.5%)s | secondss | **3392** | ✅ PASS |
| **paysim_100k.csv** | 100000 | MPI (np=16) | 0.750381s | secondss | (50.8%)s | secondss | **3392** | ✅ PASS |
| **paysim_1m.csv** | 1000000 | Sequential (1 core) | 2.240921s | secondss | 0.000000s | secondss | **31481** | ✅ PASS |
| **paysim_1m.csv** | 1000000 | MPI (np=1) | 9.034723s | secondss | (7.9%)s | secondss | **31481** | ✅ PASS |
| **paysim_1m.csv** | 1000000 | MPI (np=2) | 8.281912s | secondss | (12.6%)s | secondss | **31481** | ✅ PASS |
| **paysim_1m.csv** | 1000000 | MPI (np=4) | 6.724316s | secondss | (26.4%)s | secondss | **31481** | ✅ PASS |
| **paysim_1m.csv** | 1000000 | MPI (np=8) | 6.986199s | secondss | (25.9%)s | secondss | **31481** | ✅ PASS |
| **paysim_1m.csv** | 1000000 | MPI (np=16) | 8.498370s | secondss | (30.8%)s | secondss | **31481** | ✅ PASS |
| **paysim_full.csv** | 6362620 | Sequential (1 core) | 17.335286s | secondss | 0.000000s | secondss | **81410** | ✅ PASS |
| **paysim_full.csv** | 6362620 | MPI (np=1) | 80.068643s | secondss | (6.5%)s | secondss | **81410** | ✅ PASS |
| **paysim_full.csv** | 6362620 | MPI (np=2) | 55.786877s | secondss | (16.8%)s | secondss | **81410** | ✅ PASS |
| **paysim_full.csv** | 6362620 | MPI (np=4) | 55.584211s | secondss | (33.2%)s | secondss | **81410** | ✅ PASS |
| **paysim_full.csv** | 6362620 | MPI (np=8) | 52.977074s | secondss | (19.3%)s | secondss | **81410** | ✅ PASS |
| **paysim_full.csv** | 6362620 | MPI (np=16) | 95.667265s | secondss | (16.8%)s | secondss | **81410** | ✅ PASS |
