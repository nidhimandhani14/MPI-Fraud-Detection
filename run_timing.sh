#!/usr/bin/env bash
cd /mnt/c/Users/ESHITA/OneDrive/Desktop/MPI/src
DATA="/mnt/c/Users/ESHITA/OneDrive/Desktop/MPI/data"

extract() { grep 'MPI Total' | awk '{print $(NF-1)}'; }

echo "=== TIMING TABLE (MPI Total Comp+Comm, seconds) ==="
printf "%-10s  %10s  %10s  %10s  %10s  %10s\n" "Dataset" "np=1" "np=2" "np=4" "np=8" "np=16"
echo "----------  ----------  ----------  ----------  ----------  ----------"

for LABEL in "10k" "100k" "1m" "full"; do
    case $LABEL in
        10k)  CSV="$DATA/paysim_10k.csv"  ;;
        100k) CSV="$DATA/paysim_100k.csv" ;;
        1m)   CSV="$DATA/paysim_1m.csv"   ;;
        full) CSV="$DATA/paysim_full.csv" ;;
    esac
    T1=$(mpirun --allow-run-as-root --oversubscribe -np 1 ./mpi_fraud "$CSV" 2>/dev/null | tail -n 30 | grep 'MPI Total' | awk '{print $(NF-1)}')
    T2=$(mpirun --allow-run-as-root --oversubscribe -np 2 ./mpi_fraud "$CSV" 2>/dev/null | tail -n 30 | grep 'MPI Total' | awk '{print $(NF-1)}')
    T4=$(mpirun --allow-run-as-root --oversubscribe -np 4 ./mpi_fraud "$CSV" 2>/dev/null | tail -n 30 | grep 'MPI Total' | awk '{print $(NF-1)}')
    T8=$(mpirun --allow-run-as-root --oversubscribe -np 8 ./mpi_fraud "$CSV" 2>/dev/null | tail -n 30 | grep 'MPI Total' | awk '{print $(NF-1)}')
    T16=$(mpirun --allow-run-as-root --oversubscribe -np 16 ./mpi_fraud "$CSV" 2>/dev/null | tail -n 30 | grep 'MPI Total' | awk '{print $(NF-1)}')
    printf "%-10s  %10s  %10s  %10s  %10s  %10s\n" "$LABEL" "$T1" "$T2" "$T4" "$T8" "$T16"
done
