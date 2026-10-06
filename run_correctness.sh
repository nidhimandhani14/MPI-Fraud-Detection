#!/usr/bin/env bash
set -e
cd /mnt/c/Users/ESHITA/OneDrive/Desktop/MPI/src
DATA="/mnt/c/Users/ESHITA/OneDrive/Desktop/MPI/data"

run_check() {
    LABEL=$1; CSV=$2
    echo "=============================="
    echo " Dataset: $LABEL"
    echo "=============================="
    SEQ=$(./seq_fraud "$CSV" 2>/dev/null | tail -n 30 | grep 'Total Suspicious' | awk '{print $NF}')
    M1=$(mpirun --allow-run-as-root --oversubscribe -np 1 ./mpi_fraud "$CSV" 2>/dev/null | tail -n 30 | grep 'Total Suspicious' | awk '{print $NF}')
    M2=$(mpirun --allow-run-as-root --oversubscribe -np 2 ./mpi_fraud "$CSV" 2>/dev/null | tail -n 30 | grep 'Total Suspicious' | awk '{print $NF}')
    M4=$(mpirun --allow-run-as-root --oversubscribe -np 4 ./mpi_fraud "$CSV" 2>/dev/null | tail -n 30 | grep 'Total Suspicious' | awk '{print $NF}')
    M8=$(mpirun --allow-run-as-root --oversubscribe -np 8 ./mpi_fraud "$CSV" 2>/dev/null | tail -n 30 | grep 'Total Suspicious' | awk '{print $NF}')
    M16=$(mpirun --allow-run-as-root --oversubscribe -np 16 ./mpi_fraud "$CSV" 2>/dev/null | tail -n 30 | grep 'Total Suspicious' | awk '{print $NF}')
    echo " seq=$SEQ  np1=$M1  np2=$M2  np4=$M4  np8=$M8  np16=$M16"
    if [ "$SEQ" = "$M1" ] && [ "$SEQ" = "$M2" ] && [ "$SEQ" = "$M4" ] && [ "$SEQ" = "$M8" ] && [ "$SEQ" = "$M16" ]; then
        echo " [MATCH]"
    else
        echo " [MISMATCH]"
    fi
}

run_check "10k"  "$DATA/paysim_10k.csv"
run_check "100k" "$DATA/paysim_100k.csv"
run_check "1m"   "$DATA/paysim_1m.csv"
run_check "full" "$DATA/paysim_full.csv"
