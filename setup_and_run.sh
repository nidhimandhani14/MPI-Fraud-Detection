#!/usr/bin/env bash
# =============================================================================
# setup_and_run.sh  –  WSL setup, compile, run, and correctness check
# =============================================================================
# Run this script from INSIDE WSL:
#   cd /mnt/c/Users/ESHITA/OneDrive/Desktop/MPI
#   bash setup_and_run.sh
# =============================================================================

set -e   # exit on first error

PROJECT_DIR="$(cd "$(dirname "$0")" && pwd)"
SRC_DIR="$PROJECT_DIR/src"
DATA_DIR="$PROJECT_DIR/data"
RESULTS_DIR="$PROJECT_DIR/results"
CSV="${1:-$DATA_DIR/paysim_10k.csv}"
if [ ! -f "$CSV" ] && [ -f "$DATA_DIR/transactions.csv" ]; then
    CSV="$DATA_DIR/transactions.csv"
fi

echo "============================================================"
echo " STEP 1: Check / Install OpenMPI"
echo "============================================================"

install_if_missing() {
    if ! command -v "$1" &>/dev/null; then
        echo "  '$1' not found — installing OpenMPI..."
        sudo apt-get update -qq
        sudo apt-get install -y openmpi-bin openmpi-doc libopenmpi-dev gcc > /dev/null
    fi
}

install_if_missing mpicc
install_if_missing mpirun
install_if_missing gcc

echo "  mpicc  : $(mpicc  --version 2>&1 | head -1)"
echo "  mpirun : $(mpirun --version 2>&1 | head -1)"
echo "  nproc  : $(nproc) CPU cores available in WSL"
echo ""

# Determine if we need --allow-run-as-root
MPIRUN_EXTRA=""
if [ "$(id -u)" = "0" ]; then
    MPIRUN_EXTRA="--allow-run-as-root"
    echo "  (Running as root — will add --allow-run-as-root to mpirun)"
fi

echo "============================================================"
echo " STEP 6: Compile"
echo "============================================================"

cd "$SRC_DIR"

echo "  Compiling seq_fraud.c ..."
echo "  CMD: gcc -O2 -o seq_fraud seq_fraud.c -lm"
gcc -O2 -o seq_fraud seq_fraud.c -lm
echo "  → seq_fraud compiled OK"

echo "  Compiling mpi_fraud.c ..."
echo "  CMD: mpicc -O2 -o mpi_fraud mpi_fraud.c -lm"
mpicc -O2 -o mpi_fraud mpi_fraud.c -lm
echo "  → mpi_fraud compiled OK"
echo ""

echo "============================================================"
echo " STEP 7: Run All Versions"
echo "============================================================"
echo ""

get_count() {
    local text="$1"
    local cnt=$(echo "$text" | grep -E "Total Suspicious|Total fraud" | grep -oP '[0-9]+' | tail -1)
    echo "$cnt"
}

# ── Sequential ──────────────────────────────────────────────────────────────
echo "------------------------------------------------------------"
echo " Sequential (seq_fraud)"
echo "------------------------------------------------------------"
SEQ_OUT=$(./seq_fraud "$CSV")
echo "$SEQ_OUT"
echo ""

SEQ_COUNT=$(get_count "$SEQ_OUT")

# ── MPI np=1 ─────────────────────────────────────────────────────────────────
echo "------------------------------------------------------------"
echo " MPI  (np=1)"
echo "------------------------------------------------------------"
MPI1_OUT=$(mpirun $MPIRUN_EXTRA -np 1 ./mpi_fraud "$CSV" 2>&1)
echo "$MPI1_OUT"
echo ""

MPI1_COUNT=$(get_count "$MPI1_OUT")

# ── MPI np=2 ─────────────────────────────────────────────────────────────────
echo "------------------------------------------------------------"
echo " MPI  (np=2)"
echo "------------------------------------------------------------"
MPI2_OUT=$(mpirun $MPIRUN_EXTRA -np 2 ./mpi_fraud "$CSV" 2>&1)
echo "$MPI2_OUT"
echo ""

MPI2_COUNT=$(get_count "$MPI2_OUT")

# ── MPI np=4 ─────────────────────────────────────────────────────────────────
echo "------------------------------------------------------------"
echo " MPI  (np=4)"
echo "------------------------------------------------------------"
MPI4_OUT=$(mpirun $MPIRUN_EXTRA -np 4 ./mpi_fraud "$CSV" 2>&1)
echo "$MPI4_OUT"
echo ""

MPI4_COUNT=$(get_count "$MPI4_OUT")

echo "============================================================"
echo " STEP 8: Correctness Check"
echo "============================================================"
echo ""
echo "  Sequential count : $SEQ_COUNT"
echo "  MPI np=1 count   : $MPI1_COUNT"
echo "  MPI np=2 count   : $MPI2_COUNT"
echo "  MPI np=4 count   : $MPI4_COUNT"
echo ""

PASS=true
check() {
    local label="$1"; local a="$2"; local b="$3"
    if [ "$a" = "$b" ]; then
        echo "  [PASS] $label matches sequential ✓"
    else
        echo "  [FAIL] $label MISMATCH! seq='$a'  mpi='$b'"
        PASS=false
    fi
}

check "MPI np=1 count" "$SEQ_COUNT" "$MPI1_COUNT"
check "MPI np=2 count" "$SEQ_COUNT" "$MPI2_COUNT"
check "MPI np=4 count" "$SEQ_COUNT" "$MPI4_COUNT"

echo ""

if $PASS; then
    echo "  ✅ ALL MPI RUNS MATCH SEQUENTIAL — correctness VERIFIED"
else
    echo "  ❌ MISMATCH DETECTED — review output above"
fi

# ── Save timing summary ──────────────────────────────────────────────────────
mkdir -p "$RESULTS_DIR"
TIMING_LOG="$RESULTS_DIR/timing_log.txt"
{
    echo "Fraud Detection Timing Log — $(date)"
    echo "Dataset: $CSV"
    echo ""
    echo "Sequential:"
    echo "$SEQ_OUT" | grep TIMING
    echo ""
    echo "MPI np=1:"
    echo "$MPI1_OUT" | grep TIMING
    echo ""
    echo "MPI np=2:"
    echo "$MPI2_OUT" | grep TIMING
    echo ""
    echo "MPI np=4:"
    echo "$MPI4_OUT" | grep TIMING
    echo ""
    echo "Correctness: $(if $PASS; then echo PASS; else echo FAIL; fi)"
} > "$TIMING_LOG"

echo ""
echo "  Timing log saved to: $TIMING_LOG"
echo ""
echo "============================================================"
echo " DONE"
echo "============================================================"
