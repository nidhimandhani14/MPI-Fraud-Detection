| Dataset | Rows | Version / Procs | File I/O Time (s) | Pure Comp Time (s) | Comm Time (s) | Total Time (s) | Speedup (Comp) | Flagged | Correctness |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **paysim_10k.csv** | 10,000 | Sequential (1 core) | 0.023130s | 0.001861s | 0.000000s | 0.001861s | 1.00x | **242** | ✅ PASS |
| **paysim_10k.csv** | 10,000 | MPI Scatterv (np=1) | 0.071474s | 0.002892s | 0.000497s | 0.003389s | **0.64x** | **242** | ✅ PASS |
| **paysim_10k.csv** | 10,000 | MPI Scatterv (np=2) | 0.068887s | 0.001793s | 0.000691s | 0.002484s | **1.04x** | **242** | ✅ PASS |
| **paysim_10k.csv** | 10,000 | MPI Scatterv (np=4) | 0.068257s | 0.001621s | 0.000929s | 0.002549s | **1.15x** | **242** | ✅ PASS |
| **paysim_10k.csv** | 10,000 | MPI Scatterv (np=8) | 0.132780s | 0.001925s | 0.001657s | 0.003582s | **0.97x** | **242** | ✅ PASS |
| **paysim_10k.csv** | 10,000 | MPI Scatterv (np=16) | 0.139205s | 0.004686s | 0.109775s | 0.114461s | **0.40x** | **242** | ✅ PASS |
| **paysim_100k.csv** | 100,000 | Sequential (1 core) | 0.297243s | 0.016729s | 0.000000s | 0.016729s | 1.00x | **3,392** | ✅ PASS |
| **paysim_100k.csv** | 100,000 | MPI Scatterv (np=1) | 0.907849s | 0.031650s | 0.005674s | 0.037325s | **0.53x** | **3,392** | ✅ PASS |
| **paysim_100k.csv** | 100,000 | MPI Scatterv (np=2) | 0.876270s | 0.026821s | 0.008458s | 0.035278s | **0.62x** | **3,392** | ✅ PASS |
| **paysim_100k.csv** | 100,000 | MPI Scatterv (np=4) | 0.874158s | 0.023607s | 0.007015s | 0.030623s | **0.71x** | **3,392** | ✅ PASS |
| **paysim_100k.csv** | 100,000 | MPI Scatterv (np=8) | 1.049308s | 0.017862s | 0.011432s | 0.029295s | **0.94x** | **3,392** | ✅ PASS |
| **paysim_100k.csv** | 100,000 | MPI Scatterv (np=16) | 1.707447s | 0.025119s | 0.302789s | 0.327908s | **0.67x** | **3,392** | ✅ PASS |
| **paysim_1m.csv** | 1,000,000 | Sequential (1 core) | 2.724415s | 0.196698s | 0.000000s | 0.196698s | 1.00x | **31,481** | ✅ PASS |
| **paysim_1m.csv** | 1,000,000 | MPI Scatterv (np=1) | 8.051133s | 0.399595s | 0.060129s | 0.459724s | **0.49x** | **31,481** | ✅ PASS |
| **paysim_1m.csv** | 1,000,000 | MPI Scatterv (np=2) | 9.773480s | 0.690729s | 0.150921s | 0.841650s | **0.28x** | **31,481** | ✅ PASS |
| **paysim_1m.csv** | 1,000,000 | MPI Scatterv (np=4) | 16.954735s | 0.499581s | 0.100733s | 0.600314s | **0.39x** | **31,481** | ✅ PASS |
| **paysim_1m.csv** | 1,000,000 | MPI Scatterv (np=8) | 14.861703s | 0.629085s | 0.144792s | 0.773877s | **0.31x** | **31,481** | ✅ PASS |
| **paysim_1m.csv** | 1,000,000 | MPI Scatterv (np=16) | 16.632144s | 0.403711s | 0.215535s | 0.619246s | **0.49x** | **31,481** | ✅ PASS |
| **paysim_full.csv** | 6,362,620 | Sequential (1 core) | 19.704094s | 1.118921s | 0.000000s | 1.118921s | 1.00x | **81,410** | ✅ PASS |
| **paysim_full.csv** | 6,362,620 | MPI Scatterv (np=1) | 50.309191s | 3.819384s | 0.364101s | 4.183486s | **0.29x** | **81,410** | ✅ PASS |
| **paysim_full.csv** | 6,362,620 | MPI Scatterv (np=2) | 43.316281s | 1.339125s | 0.325050s | 1.664175s | **0.84x** | **81,410** | ✅ PASS |
| **paysim_full.csv** | 6,362,620 | MPI Scatterv (np=4) | 40.661291s | 1.646074s | 0.391414s | 2.037488s | **0.68x** | **81,410** | ✅ PASS |
| **paysim_full.csv** | 6,362,620 | MPI Scatterv (np=8) | 55.971555s | 2.426079s | 0.496429s | 2.922508s | **0.46x** | **81,410** | ✅ PASS |
| **paysim_full.csv** | 6,362,620 | MPI Scatterv (np=16) | 117.029103s | 5.494312s | 1.070708s | 6.565019s | **0.20x** | **81,410** | ✅ PASS |
