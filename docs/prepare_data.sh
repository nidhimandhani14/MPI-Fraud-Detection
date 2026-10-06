#!/usr/bin/env bash
# =============================================================================
# prepare_data.sh  —  Steps 1-3: clean old data, copy PaySim, create slices
# =============================================================================
set -e

PAYSIM_SRC="/mnt/c/Users/ESHITA/Downloads/archive (3)/PS_20174392719_1491204439457_log.csv"
PROJECT="/mnt/c/Users/ESHITA/OneDrive/Desktop/MPI"
DATA="$PROJECT/data"

echo "============================================================"
echo " STEP 1: Remove old data files"
echo "============================================================"
cd "$DATA"
for f in transactions.csv creditcard.csv creditcard_full.csv; do
    if [ -f "$f" ]; then
        rm -f "$f"
        echo "  Removed: $f"
    fi
done
echo "  Old data cleaned."
echo ""

echo "============================================================"
echo " STEP 2: Copy PaySim CSV → data/paysim_full.csv"
echo "============================================================"
if [ ! -f "$DATA/paysim_full.csv" ]; then
    echo "  Copying 471 MB file (this may take ~10-20s)..."
    cp "$PAYSIM_SRC" "$DATA/paysim_full.csv"
    echo "  Done: $(ls -lh $DATA/paysim_full.csv | awk '{print $5}') copied"
else
    echo "  paysim_full.csv already exists — skipping copy."
fi
echo ""

echo "============================================================"
echo " STEP 3: Create slice files (head-based, fast)"
echo "============================================================"
HEADER=$(head -1 "$DATA/paysim_full.csv")

make_slice() {
    local name="$1"
    local rows="$2"
    local out="$DATA/$name"
    if [ ! -f "$out" ]; then
        echo "  Creating $name ($rows data rows)..."
        echo "$HEADER" > "$out"
        tail -n +2 "$DATA/paysim_full.csv" | head -n "$rows" >> "$out"
        echo "  Done: $(wc -l < $out) lines (incl. header)"
    else
        echo "  $name already exists — skipping."
    fi
}

make_slice "paysim_10k.csv"   10000
make_slice "paysim_100k.csv"  100000
make_slice "paysim_1m.csv"    1000000

echo ""
echo "============================================================"
echo " STEP 4: Confirm header + first 3 rows of paysim_full.csv"
echo "============================================================"
echo "  Header + rows 1-3:"
head -4 "$DATA/paysim_full.csv"
echo ""
echo "  Files in data/:"
ls -lh "$DATA/"

echo ""
echo "DONE — data ready."
