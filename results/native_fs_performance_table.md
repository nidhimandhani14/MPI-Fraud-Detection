| Dataset | Rows | Version / Procs | File I/O Time (s) | Pure Comp Time (s) | Comm Time (s) | Total Comp+Comm (s) | Pure Comp Speedup | Flagged | Correctness |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **paysim_10k.csv** | 10,000 | Sequential (1 core) | 0.009703s | 0.000721s | 0.000000s | 0.000721s | 1.00x | **242** | ✅ PASS |
| **paysim_10k.csv** | 10,000 | MPI Parallel I/O (np=1) | 0.011363s | 0.001471s | 0.000042s | 0.001513s | **0.49x** | **242** | ✅ PASS |
| **paysim_10k.csv** | 10,000 | MPI Parallel I/O (np=2) | 0.005185s | 0.000811s | 0.000058s | 0.000869s | **0.89x** | **242** | ✅ PASS |
| **paysim_10k.csv** | 10,000 | MPI Parallel I/O (np=4) | 0.025983s | 0.001292s | 0.018247s | 0.019538s | **0.56x** | **242** | ✅ PASS |
| **paysim_100k.csv** | 100,000 | Sequential (1 core) | 0.101286s | 0.008798s | 0.000000s | 0.008798s | 1.00x | **3,392** | ✅ PASS |
| **paysim_100k.csv** | 100,000 | MPI Parallel I/O (np=1) | 0.104775s | 0.009996s | 0.000496s | 0.010492s | **0.88x** | **3,392** | ✅ PASS |
| **paysim_100k.csv** | 100,000 | MPI Parallel I/O (np=2) | 0.080857s | 0.015992s | 0.000579s | 0.016571s | **0.55x** | **3,392** | ✅ PASS |
| **paysim_100k.csv** | 100,000 | MPI Parallel I/O (np=4) | 0.060045s | 0.013316s | 0.001303s | 0.014620s | **0.66x** | **3,392** | ✅ PASS |
| **paysim_1m.csv** | 1,000,000 | Sequential (1 core) | 1.063409s | 0.110947s | 0.000000s | 0.110947s | 1.00x | **31,481** | ✅ PASS |
| **paysim_1m.csv** | 1,000,000 | MPI Parallel I/O (np=1) | 0.974001s | 0.112388s | 0.005946s | 0.118334s | **0.99x** | **31,481** | ✅ PASS |
| **paysim_1m.csv** | 1,000,000 | MPI Parallel I/O (np=2) | 0.685608s | 0.139042s | 0.009876s | 0.148918s | **0.80x** | **31,481** | ✅ PASS |
| **paysim_1m.csv** | 1,000,000 | MPI Parallel I/O (np=4) | 0.485683s | 0.160981s | 0.010135s | 0.171116s | **0.69x** | **31,481** | ✅ PASS |
| **paysim_full.csv** | 6,362,620 | Sequential (1 core) | 7.241339s | 0.964967s | 0.000000s | 0.964967s | 1.00x | **81,410** | ✅ PASS |
| **paysim_full.csv** | 6,362,620 | MPI Parallel I/O (np=1) | 7.382291s | 0.886887s | 0.033565s | 0.920451s | **1.09x** | **81,410** | ✅ PASS |
| **paysim_full.csv** | 6,362,620 | MPI Parallel I/O (np=2) | 3.603505s | 0.889127s | 0.028170s | 0.917296s | **1.09x** | **81,410** | ✅ PASS |
| **paysim_full.csv** | 6,362,620 | MPI Parallel I/O (np=4) | 3.177566s | 1.273878s | 0.057637s | 1.331515s | **0.76x** | **81,410** | ✅ PASS |
