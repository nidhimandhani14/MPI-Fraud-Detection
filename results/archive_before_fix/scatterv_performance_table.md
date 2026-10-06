| Dataset | Rows | Version / Procs | File I/O Time (s) | Pure Comp Time (s) | Comm Time (s) | Total Time (s) | Pure Comp Speedup | Flagged | Correctness |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **paysim_10k.csv** | 10,000 | Sequential (1 core) | 0.026371s | 0.000938s | 0.000000s | 0.000938s | 1.00x | **242** | ✅ PASS |
| **paysim_10k.csv** | 10,000 | MPI Scatterv (np=1) | 0.063054s | 0.001231s | 0.000079s | 0.001310s | **0.76x** | **242** | ✅ PASS |
| **paysim_10k.csv** | 10,000 | MPI Scatterv (np=2) | 0.108450s | 0.001273s | 0.000200s | 0.001473s | **0.74x** | **242** | ✅ PASS |
| **paysim_10k.csv** | 10,000 | MPI Scatterv (np=4) | 0.082658s | 0.001296s | 0.000325s | 0.001621s | **0.72x** | **242** | ✅ PASS |
| **paysim_10k.csv** | 10,000 | MPI Scatterv (np=8) | 0.084939s | 0.001330s | 0.000599s | 0.001929s | **0.71x** | **242** | ✅ PASS |
| **paysim_10k.csv** | 10,000 | MPI Scatterv (np=16) | 0.198794s | 0.004585s | 0.015070s | 0.019656s | **0.20x** | **242** | ✅ PASS |
| **paysim_100k.csv** | 100,000 | Sequential (1 core) | 0.242144s | 0.017677s | 0.000000s | 0.017677s | 1.00x | **3,392** | ✅ PASS |
| **paysim_100k.csv** | 100,000 | MPI Scatterv (np=1) | 0.721685s | 0.015664s | 0.001363s | 0.017027s | **1.13x** | **3,392** | ✅ PASS |
| **paysim_100k.csv** | 100,000 | MPI Scatterv (np=2) | 0.796785s | 0.011783s | 0.001716s | 0.013499s | **1.50x** | **3,392** | ✅ PASS |
| **paysim_100k.csv** | 100,000 | MPI Scatterv (np=4) | 0.788233s | 0.011695s | 0.004497s | 0.016192s | **1.51x** | **3,392** | ✅ PASS |
| **paysim_100k.csv** | 100,000 | MPI Scatterv (np=8) | 0.812304s | 0.022913s | 0.002764s | 0.025677s | **0.77x** | **3,392** | ✅ PASS |
| **paysim_100k.csv** | 100,000 | MPI Scatterv (np=16) | 0.908526s | 0.022287s | 0.005754s | 0.028040s | **0.79x** | **3,392** | ✅ PASS |
| **paysim_1m.csv** | 1,000,000 | Sequential (1 core) | 2.105645s | 0.177538s | 0.000000s | 0.177538s | 1.00x | **31,481** | ✅ PASS |
| **paysim_1m.csv** | 1,000,000 | MPI Scatterv (np=1) | 8.502199s | 0.191028s | 0.018232s | 0.209259s | **0.93x** | **31,481** | ✅ PASS |
| **paysim_1m.csv** | 1,000,000 | MPI Scatterv (np=2) | 7.345753s | 0.135442s | 0.016046s | 0.151488s | **1.31x** | **31,481** | ✅ PASS |
| **paysim_1m.csv** | 1,000,000 | MPI Scatterv (np=4) | 6.901162s | 0.143984s | 0.022176s | 0.166159s | **1.23x** | **31,481** | ✅ PASS |
| **paysim_1m.csv** | 1,000,000 | MPI Scatterv (np=8) | 6.486933s | 0.182055s | 0.021132s | 0.203187s | **0.98x** | **31,481** | ✅ PASS |
| **paysim_1m.csv** | 1,000,000 | MPI Scatterv (np=16) | 8.817006s | 0.262410s | 0.030760s | 0.293170s | **0.68x** | **31,481** | ✅ PASS |
| **paysim_full.csv** | 6,362,620 | Sequential (1 core) | 13.298448s | 1.495859s | 0.000000s | 1.495859s | 1.00x | **81,410** | ✅ PASS |
| **paysim_full.csv** | 6,362,620 | MPI Scatterv (np=1) | 43.662129s | 1.162373s | 0.106320s | 1.268693s | **1.29x** | **81,410** | ✅ PASS |
| **paysim_full.csv** | 6,362,620 | MPI Scatterv (np=2) | 44.731764s | 1.102501s | 0.116526s | 1.219027s | **1.36x** | **81,410** | ✅ PASS |
| **paysim_full.csv** | 6,362,620 | MPI Scatterv (np=4) | 42.716295s | 1.010649s | 0.112804s | 1.123452s | **1.48x** | **81,410** | ✅ PASS |
| **paysim_full.csv** | 6,362,620 | MPI Scatterv (np=8) | 42.109382s | 1.577960s | 0.147245s | 1.725205s | **0.95x** | **81,410** | ✅ PASS |
| **paysim_full.csv** | 6,362,620 | MPI Scatterv (np=16) | 72.019273s | 2.185188s | 0.235600s | 2.420788s | **0.68x** | **81,410** | ✅ PASS |
