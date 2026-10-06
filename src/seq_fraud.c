/*
 * seq_fraud.c – Sequential Fraud Detection on PaySim Dataset
 * -----------------------------------------------------------
 * Parses PaySim CSV format:
 *   step,type,amount,nameOrig,oldbalanceOrg,newbalanceOrig,
 *   nameDest,oldbalanceDest,newbalanceDest,isFraud,isFlaggedFraud
 *
 * Rules for flagging transaction:
 *   1. Amount rule: amount > mean + 2*stddev
 *   2. Velocity rule: account (nameOrig) total transaction count > VELOCITY_THRESHOLD
 *
 * Compile:
 *   gcc -O2 -o seq_fraud seq_fraud.c -lm
 *
 * Run:
 *   ./seq_fraud ../data/paysim_10k.csv
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <stdint.h>

#define VELOCITY_THRESHOLD 10
#define INITIAL_CAPACITY   100000
#define LINE_BUF           512

typedef struct {
    int      id;            /* 1-based row index */
    int      step;          /* time / step column */
    double   amount;        /* transaction amount */
    char     nameOrig[32];  /* originator account string */
    uint64_t account_id;    /* 64-bit hash of nameOrig */
    int      is_fraud_gt;   /* ground truth isFraud */
} Transaction;

/* Hash Table for Account Velocity Tracking */
typedef struct {
    uint64_t account_id;
    int count;
} AccountEntry;

typedef struct {
    AccountEntry *entries;
    size_t capacity;
    size_t count;
} HashTable;

static uint64_t hash_string(const char *str) {
    uint64_t hash = 14695981039346656037ULL;
    while (*str) {
        hash ^= (unsigned char)*str++;
        hash *= 1099511628211ULL;
    }
    return hash;
}

static void hash_table_init(HashTable *ht, size_t initial_capacity) {
    ht->capacity = initial_capacity;
    ht->count = 0;
    ht->entries = (AccountEntry *)calloc(initial_capacity, sizeof(AccountEntry));
}

static void hash_table_free(HashTable *ht) {
    free(ht->entries);
}

static void hash_table_add(HashTable *ht, uint64_t account_id, int add_count) {
    if (ht->count * 2 >= ht->capacity) {
        size_t new_cap = ht->capacity * 2;
        AccountEntry *old_entries = ht->entries;
        size_t old_cap = ht->capacity;

        ht->entries = (AccountEntry *)calloc(new_cap, sizeof(AccountEntry));
        ht->capacity = new_cap;
        ht->count = 0;

        for (size_t i = 0; i < old_cap; i++) {
            if (old_entries[i].account_id != 0) {
                uint64_t id = old_entries[i].account_id;
                int cnt = old_entries[i].count;
                size_t idx = id % new_cap;
                while (ht->entries[idx].account_id != 0) {
                    idx = (idx + 1) % new_cap;
                }
                ht->entries[idx].account_id = id;
                ht->entries[idx].count = cnt;
                ht->count++;
            }
        }
        free(old_entries);
    }

    size_t idx = account_id % ht->capacity;
    while (ht->entries[idx].account_id != 0 && ht->entries[idx].account_id != account_id) {
        idx = (idx + 1) % ht->capacity;
    }
    if (ht->entries[idx].account_id == 0) {
        ht->entries[idx].account_id = account_id;
        ht->entries[idx].count = add_count;
        ht->count++;
    } else {
        ht->entries[idx].count += add_count;
    }
}

static int hash_table_get(HashTable *ht, uint64_t account_id) {
    if (ht->capacity == 0) return 0;
    size_t idx = account_id % ht->capacity;
    size_t start_idx = idx;
    while (ht->entries[idx].account_id != 0) {
        if (ht->entries[idx].account_id == account_id) {
            return ht->entries[idx].count;
        }
        idx = (idx + 1) % ht->capacity;
        if (idx == start_idx) break;
    }
    return 0;
}

/* CSV Reader with dynamic reallocation */
static Transaction *load_csv(const char *path, int *out_count) {
    FILE *fp = fopen(path, "r");
    if (!fp) {
        fprintf(stderr, "ERROR: Cannot open '%s'\n", path);
        return NULL;
    }

    size_t capacity = INITIAL_CAPACITY;
    int count = 0;
    Transaction *txns = (Transaction *)malloc(capacity * sizeof(Transaction));
    if (!txns) {
        fclose(fp);
        return NULL;
    }

    char line[LINE_BUF];
    /* skip header */
    if (!fgets(line, sizeof(line), fp)) {
        fclose(fp);
        *out_count = 0;
        return txns;
    }

    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\r\n")] = '\0';
        if (strlen(line) == 0) continue;

        if (count >= capacity) {
            capacity *= 2;
            Transaction *new_txns = (Transaction *)realloc(txns, capacity * sizeof(Transaction));
            if (!new_txns) {
                fprintf(stderr, "Out of memory loading CSV\n");
                free(txns);
                fclose(fp);
                return NULL;
            }
            txns = new_txns;
        }

        /* Parse PaySim 11 columns */
        int step, isFraud, isFlaggedFraud;
        char type[32], nameOrig[32], nameDest[32];
        double amount, oldbalanceOrg, newbalanceOrig, oldbalanceDest, newbalanceDest;

        if (sscanf(line, "%d,%31[^,],%lf,%31[^,],%lf,%lf,%31[^,],%lf,%lf,%d,%d",
                   &step, type, &amount, nameOrig, &oldbalanceOrg, &newbalanceOrig,
                   nameDest, &oldbalanceDest, &newbalanceDest, &isFraud, &isFlaggedFraud) >= 10) {
            count++;
            txns[count - 1].id = count;
            txns[count - 1].step = step;
            txns[count - 1].amount = amount;
            strncpy(txns[count - 1].nameOrig, nameOrig, sizeof(txns[count - 1].nameOrig) - 1);
            txns[count - 1].nameOrig[sizeof(txns[count - 1].nameOrig) - 1] = '\0';
            txns[count - 1].account_id = hash_string(nameOrig);
            txns[count - 1].is_fraud_gt = isFraud;
        }
    }

    fclose(fp);
    *out_count = count;
    return txns;
}

int main(int argc, char *argv[]) {
    const char *csv_path = (argc > 1) ? argv[1] : "../data/paysim_10k.csv";

    double io_start = (double)clock() / CLOCKS_PER_SEC;
    int n = 0;
    Transaction *txns = load_csv(csv_path, &n);
    double io_end = (double)clock() / CLOCKS_PER_SEC;

    if (!txns || n <= 0) {
        fprintf(stderr, "Failed to load transactions from %s\n", csv_path);
        return 1;
    }

    printf("Loaded %d transactions from '%s' (I/O time: %.6f s)\n\n", n, csv_path, io_end - io_start);

    /* ================================================================
     * BEGIN TIMED COMPUTATION
     * ================================================================ */
    clock_t t_start = clock();

    /* Pass 1: Sum amounts & build account counts */
    double sum = 0.0;
    HashTable ht;
    hash_table_init(&ht, n < 1000 ? 1000 : n);

    for (int i = 0; i < n; i++) {
        sum += txns[i].amount;
        hash_table_add(&ht, txns[i].account_id, 1);
    }
    double mean = sum / n;

    /* Pass 2: Sum squared deviations */
    double sq_sum = 0.0;
    for (int i = 0; i < n; i++) {
        double diff = txns[i].amount - mean;
        sq_sum += diff * diff;
    }
    double stddev    = sqrt(sq_sum / n);
    double threshold = mean + 2.0 * stddev;

    /* Pass 3: Flag suspicious transactions */
    int suspicious_count = 0;
    int amount_flag_count = 0;
    int velocity_flag_count = 0;

    printf("Flagged Suspicious Transactions (first 20 printed):\n");
    printf("%-8s  %-6s  %-14s  %-15s  %-12s  %-10s\n", "RowID", "Step", "Amount($)", "Account", "GT_IsFraud", "Reason");
    printf("--------  ------  --------------  ---------------  ------------  ----------\n");

    for (int i = 0; i < n; i++) {
        int amount_rule   = (txns[i].amount > threshold);
        int acct_count    = hash_table_get(&ht, txns[i].account_id);
        int velocity_rule = (acct_count > VELOCITY_THRESHOLD);

        if (amount_rule) amount_flag_count++;
        if (velocity_rule) velocity_flag_count++;

        if (amount_rule || velocity_rule) {
            suspicious_count++;
            if (suspicious_count <= 20) {
                const char *reason = (amount_rule && velocity_rule) ? "BOTH" : (amount_rule ? "HIGH_AMT" : "VELOCITY");
                printf("%-8d  %-6d  %-14.2f  %-15s  %-12d  %-10s\n",
                       txns[i].id, txns[i].step, txns[i].amount, txns[i].nameOrig, txns[i].is_fraud_gt, reason);
            }
        }
    }
    if (suspicious_count > 20) {
        printf("... and %d more flagged transactions (total %d).\n", suspicious_count - 20, suspicious_count);
    }

    clock_t t_end = clock();
    /* ================================================================
     * END TIMED COMPUTATION
     * ================================================================ */

    double elapsed = (double)(t_end - t_start) / CLOCKS_PER_SEC;

    printf("\n--- Summary Statistics (Sequential) ---\n");
    printf("Total transactions   : %d\n", n);
    printf("Mean amount          : $%.2f\n", mean);
    printf("Std deviation        : $%.2f\n", stddev);
    printf("Amount Threshold     : $%.2f (mean + 2*stddev)\n", threshold);
    printf("High Amount Flagged  : %d\n", amount_flag_count);
    printf("Velocity Flagged (>%d): %d\n", VELOCITY_THRESHOLD, velocity_flag_count);
    printf("Total Suspicious     : %d\n", suspicious_count);
    printf("[TIMING] Computation time : %.6f seconds\n", elapsed);

    hash_table_free(&ht);
    free(txns);
    return 0;
}
