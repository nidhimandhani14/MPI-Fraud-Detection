#!/usr/bin/env bash
# =============================================================================
# run_tests.sh — Compile & Test Correctness on paysim_10k.csv
# =============================================================================
set -e

PROJECT="/mnt/c/Users/ESHITA/OneDrive/Desktop/MPI"
SRC="$PROJECT/src"
DATA="$PROJECT/data"
RESULTS="$PROJECT/results"

mkdir -p "$RESULTS"

cd "$SRC"

echo "============================================================"
echo " STEP 8: Compile both programs"
echo "============================================================"
echo "Compiling seq_fraud.c..."
gcc -O2 -o seq_fraud seq_fraud.c -lm
echo "→ seq_fraud compiled OK."

echo "Compiling mpi_fraud.c..."
mpicc -O2 -o mpi_fraud mpi_fraud.c -lm
echo "→ mpi_fraud compiled OK."
echo ""

echo "============================================================"
echo " STEP 9: Test Correctness on paysim_10k.csv"
echo "============================================================"

CSV_10K="$DATA/paysim_10k.csv"

echo "--- 1. Sequential Run ---"
SEQ_OUT=$(./seq_fraud "$CSV_10K")
echo "$SEQ_OUT"
echo ""

SEQ_COUNT=$(echo "$SEQ_OUT" | grep "Total Suspicious" | awk '{print $NF}')

echo "--- 2. MPI np=1 Run ---"
MPI1_OUT=$(mpirun --allow-run-as-root -np 1 ./mpi_fraud "$CSV_10K")
echo "$MPI1_OUT"
echo ""
MPI1_COUNT=$(echo "$MPI1_OUT" | grep "Total Suspicious" | awk '{print $NF}')

echo "--- 3. MPI np=2 Run ---"
MPI2_OUT=$(mpirun --allow-run-as-root -np 2 ./mpi_fraud "$CSV_10K")
echo "$MPI2_OUT"
echo ""
MPI2_COUNT=$(echo "$MPI2_OUT" | grep "Total Suspicious" | awk '{print $NF}')

echo "--- 4. MPI np=4 Run ---"
MPI4_OUT=$(mpirun --allow-run-as-root -np 4 ./mpi_fraud "$CSV_10K")
echo "$MPI4_OUT"
echo ""
MPI4_COUNT=$(echo "$MPI4_OUT" | grep "Total Suspicious" | awk '{print $NF}')

echo "============================================================"
echo " CORRECTNESS VERIFICATION (paysim_10k.csv)"
echo "============================================================"
echo "Sequential Total Suspicious: $SEQ_COUNT"
echo "MPI np=1 Total Suspicious   : $MPI1_COUNT"
echo "MPI np=2 Total Suspicious   : $MPI2_COUNT"
echo "MPI np=4 Total Suspicious   : $MPI4_COUNT"

if [ "$SEQ_COUNT" = "$MPI1_COUNT" ] && [ "$SEQ_COUNT" = "$MPI2_COUNT" ] && [ "$SEQ_COUNT" = "$MPI4_COUNT" ]; then
    echo "✅ PERFECT MATCH! Sequential and all MPI runs produce identical flagged counts ($SEQ_COUNT)."
else
    echo "❌ MISMATCH DETECTED! Check outputs above."
    exit 1
fi
