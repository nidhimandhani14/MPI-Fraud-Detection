/*
 * mpi_fraud.c – MPI Parallel Fraud Detection on PaySim Dataset (True Parallel I/O)
 * ---------------------------------------------------------------------------------
 * Parallel version of seq_fraud.c using OpenMPI with True Parallel File I/O.
 *
 * Architecture & Algorithm:
 *   1. Initial Fast Line Count: Rank 0 scans newlines quickly with a 1MB buffer,
 *      then broadcasts the total row count N to all ranks (MPI_Bcast).
 *   2. Parallel Independent File I/O:
 *      - Every rank computes its chunk range (start_row, local_n).
 *      - Every rank opens the CSV file independently, skips to start_row,
 *        and reads/parses ONLY its assigned local_n rows directly into memory.
 *      - Total I/O time is measured across all ranks via MPI_Reduce(MPI_MAX).
 *   3. Distributed Account Velocity Merging:
 *      - Each rank transmits its local account IDs to Rank 0 via MPI_Gatherv.
 *      - Rank 0 builds the global velocity hash table and identifies high-velocity accounts.
 *      - Rank 0 broadcasts the high-velocity account list to all ranks via MPI_Bcast.
 *   4. Global Reductions:
 *      - Local sum & MPI_Allreduce for global sum -> global mean
 *      - Local sq-dev & MPI_Allreduce for global sq-dev -> global stddev
 *   5. Parallel Local Flagging:
 *      - Each rank independently flags suspicious transactions in its chunk.
 *   6. MPI_Reduce sums total suspicious counts at Rank 0.
 *
 * Compile:
 *   mpicc -O2 -o mpi_fraud mpi_fraud.c -lm
 *
 * Run:
 *   mpirun -np 4 ./mpi_fraud ../data/paysim_10k.csv
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include <mpi.h>

#define VELOCITY_THRESHOLD 10
#define INITIAL_CAPACITY   100000
#define LINE_BUF           512

/* Hash table entry for account velocity tracking */
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

/* Fast line count on Rank 0 */
static int count_csv_lines(const char *path) {
    FILE *fp = fopen(path, "rb");
    if (!fp) {
        fprintf(stderr, "ERROR: Cannot open '%s' for line counting\n", path);
        return -1;
    }
    char buf[1024 * 1024]; // 1MB buffer
    size_t bytes;
    int lines = 0;
    while ((bytes = fread(buf, 1, sizeof(buf), fp)) > 0) {
        for (size_t i = 0; i < bytes; i++) {
            if (buf[i] == '\n') lines++;
        }
    }
    fclose(fp);
    /* Subtract 1 for CSV header line */
    return (lines > 0) ? (lines - 1) : 0;
}

/* Parallel CSV chunk loader for each rank */
static int load_csv_chunk(const char *path, int start_row, int my_rows,
                          int **out_steps, double **out_amounts,
                          uint64_t **out_account_ids, int **out_is_frauds, char ***out_nameOrigs) {
    FILE *fp = fopen(path, "r");
    if (!fp) {
        fprintf(stderr, "ERROR: Cannot open '%s'\n", path);
        return -1;
    }

    int *steps = (int *)malloc(my_rows * sizeof(int));
    double *amounts = (double *)malloc(my_rows * sizeof(double));
    uint64_t *account_ids = (uint64_t *)malloc(my_rows * sizeof(uint64_t));
    int *is_frauds = (int *)malloc(my_rows * sizeof(int));
    char **nameOrigs = (char **)malloc(my_rows * sizeof(char *));

    if (!steps || !amounts || !account_ids || !is_frauds || !nameOrigs) {
        fclose(fp);
        return -1;
    }

    char line[LINE_BUF];
    /* Skip header line */
    if (!fgets(line, sizeof(line), fp)) {
        fclose(fp);
        return 0;
    }

    /* Skip preceding rows before start_row */
    for (int i = 0; i < start_row; i++) {
        if (!fgets(line, sizeof(line), fp)) {
            fclose(fp);
            return 0;
        }
    }

    /* Read and parse strictly my_rows rows */
    int count = 0;
    while (count < my_rows && fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\r\n")] = '\0';
        if (strlen(line) == 0) continue;

        int step, isFraud, isFlaggedFraud;
        char type[32], nameOrig[32], nameDest[32];
        double amount, oldbalanceOrg, newbalanceOrig, oldbalanceDest, newbalanceDest;

        if (sscanf(line, "%d,%31[^,],%lf,%31[^,],%lf,%lf,%31[^,],%lf,%lf,%d,%d",
                   &step, type, &amount, nameOrig, &oldbalanceOrg, &newbalanceOrig,
                   nameDest, &oldbalanceDest, &newbalanceDest, &isFraud, &isFlaggedFraud) >= 10) {
            steps[count]       = step;
            amounts[count]     = amount;
            account_ids[count] = hash_string(nameOrig);
            is_frauds[count]   = isFraud;
            nameOrigs[count]   = strdup(nameOrig);
            count++;
        }
    }

    fclose(fp);
    *out_steps       = steps;
    *out_amounts     = amounts;
    *out_account_ids = account_ids;
    *out_is_frauds   = is_frauds;
    *out_nameOrigs   = nameOrigs;
    return count;
}

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    const char *csv_path = (argc > 1) ? argv[1] : "../data/paysim_10k.csv";

    /* ================================================================
     * STEP 1: Fast Line Count & Distribution of Range
     * ================================================================ */
    double t_io_start = MPI_Wtime();
    int n = 0;
    if (rank == 0) {
        n = count_csv_lines(csv_path);
        if (n <= 0) {
            fprintf(stderr, "Rank 0: Failed to count lines in '%s'\n", csv_path);
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
    }

    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

    int base_chunk = n / size;
    int rem        = n % size;
    int start_row  = rank * base_chunk;
    int local_n    = base_chunk + (rank == size - 1 ? rem : 0);

    /* Chunk displacements and sizes for gathering */
    int *sendcounts = (int *)malloc(size * sizeof(int));
    int *displs     = (int *)malloc(size * sizeof(int));
    for (int r = 0; r < size; r++) {
        sendcounts[r] = base_chunk + (r == size - 1 ? rem : 0);
        displs[r]     = r * base_chunk;
    }

    /* ================================================================
     * STEP 2: True Parallel CSV Chunk Loading
     * ================================================================ */
    int      *local_steps       = NULL;
    double   *local_amounts     = NULL;
    uint64_t *local_account_ids = NULL;
    int      *local_is_frauds   = NULL;
    char    **local_nameOrigs   = NULL;

    int loaded = load_csv_chunk(csv_path, start_row, local_n,
                                &local_steps, &local_amounts,
                                &local_account_ids, &local_is_frauds,
                                &local_nameOrigs);

    if (loaded != local_n) {
        fprintf(stderr, "Rank %d: Expected %d rows but parsed %d\n", rank, local_n, loaded);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    double local_io_time = MPI_Wtime() - t_io_start;
    double max_io_time   = 0.0;
    MPI_Reduce(&local_io_time, &max_io_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        printf("Rank 0: Loaded %d transactions from '%s' (I/O time: %.6f s)\n\n",
               n, csv_path, max_io_time);
    }

    /* ================================================================
     * TIMED MPI SECTION (Computation + Communication)
     * ================================================================ */
    MPI_Barrier(MPI_COMM_WORLD);
    double t_comp_start = MPI_Wtime();
    double comm_time    = 0.0;

    /* 1. Gather all Account IDs to Rank 0 for Global Velocity Analysis */
    uint64_t *global_account_ids = NULL;
    if (rank == 0) {
        global_account_ids = (uint64_t *)malloc((size_t)n * sizeof(uint64_t));
    }

    double t0 = MPI_Wtime();
    MPI_Gatherv(local_account_ids, local_n, MPI_UINT64_T,
                global_account_ids, sendcounts, displs, MPI_UINT64_T,
                0, MPI_COMM_WORLD);
    comm_time += (MPI_Wtime() - t0);

    /* 2. Rank 0 builds global velocity table & extracts high-velocity accounts */
    int       num_high_vel   = 0;
    uint64_t *high_vel_accts = NULL;

    if (rank == 0) {
        HashTable global_ht;
        hash_table_init(&global_ht, n < 1000 ? 1000 : n);
        for (int i = 0; i < n; i++) {
            hash_table_add(&global_ht, global_account_ids[i], 1);
        }

        num_high_vel = 0;
        for (size_t i = 0; i < global_ht.capacity; i++) {
            if (global_ht.entries[i].account_id != 0 &&
                global_ht.entries[i].count > VELOCITY_THRESHOLD) {
                num_high_vel++;
            }
        }
        high_vel_accts = (uint64_t *)malloc((num_high_vel > 0 ? num_high_vel : 1) * sizeof(uint64_t));
        int h_idx = 0;
        for (size_t i = 0; i < global_ht.capacity; i++) {
            if (global_ht.entries[i].account_id != 0 &&
                global_ht.entries[i].count > VELOCITY_THRESHOLD) {
                high_vel_accts[h_idx++] = global_ht.entries[i].account_id;
            }
        }
        hash_table_free(&global_ht);
        free(global_account_ids);
    }

    /* 3. Broadcast high-velocity account list to all ranks */
    t0 = MPI_Wtime();
    MPI_Bcast(&num_high_vel, 1, MPI_INT, 0, MPI_COMM_WORLD);
    if (rank != 0) {
        high_vel_accts = (uint64_t *)malloc((num_high_vel > 0 ? num_high_vel : 1) * sizeof(uint64_t));
    }
    if (num_high_vel > 0) {
        MPI_Bcast(high_vel_accts, num_high_vel, MPI_UINT64_T, 0, MPI_COMM_WORLD);
    }
    comm_time += (MPI_Wtime() - t0);

    /* Build a local fast lookup set for high-velocity accounts */
    HashTable high_vel_ht;
    hash_table_init(&high_vel_ht, num_high_vel < 100 ? 100 : num_high_vel * 2);
    for (int i = 0; i < num_high_vel; i++) {
        hash_table_add(&high_vel_ht, high_vel_accts[i], 1);
    }
    free(high_vel_accts);

    /* 4. Local Sum & Global Mean Reduction */
    double local_sum = 0.0;
    for (int i = 0; i < local_n; i++) {
        local_sum += local_amounts[i];
    }

    double global_sum = 0.0;
    t0 = MPI_Wtime();
    MPI_Allreduce(&local_sum, &global_sum, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
    comm_time += (MPI_Wtime() - t0);

    double mean = global_sum / n;

    /* 5. Local Sq Dev & Global StdDev Reduction */
    double local_sq = 0.0;
    for (int i = 0; i < local_n; i++) {
        double diff = local_amounts[i] - mean;
        local_sq += diff * diff;
    }

    double global_sq = 0.0;
    t0 = MPI_Wtime();
    MPI_Allreduce(&local_sq, &global_sq, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
    comm_time += (MPI_Wtime() - t0);

    double stddev    = sqrt(global_sq / n);
    double threshold = mean + 2.0 * stddev;

    /* 6. Local Flagging in Parallel */
    int local_suspicious    = 0;
    int local_amount_flag   = 0;
    int local_velocity_flag = 0;

    for (int i = 0; i < local_n; i++) {
        int amount_rule   = (local_amounts[i] > threshold);
        int velocity_rule = (hash_table_get(&high_vel_ht, local_account_ids[i]) > 0);

        if (amount_rule)   local_amount_flag++;
        if (velocity_rule) local_velocity_flag++;

        if (amount_rule || velocity_rule) {
            local_suspicious++;
            int global_row_id = displs[rank] + i + 1;
            if (rank == 0 && local_suspicious <= 20) {
                const char *reason = (amount_rule && velocity_rule) ? "BOTH" : (amount_rule ? "HIGH_AMT" : "VELOCITY");
                printf("[Rank %d] Row %-8d Step %-5d Amount $%-12.2f  Account %-12s GT_Fraud %-2d  [%s]\n",
                       rank, global_row_id, local_steps[i], local_amounts[i],
                       local_nameOrigs ? local_nameOrigs[i] : "N/A", local_is_frauds[i], reason);
            }
        }
    }

    hash_table_free(&high_vel_ht);

    /* 7. Sum Suspicious Counts at Rank 0 */
    int total_suspicious = 0, total_amount_flags = 0, total_velocity_flags = 0;
    t0 = MPI_Wtime();
    MPI_Reduce(&local_suspicious,    &total_suspicious,    1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&local_amount_flag,   &total_amount_flags,   1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    MPI_Reduce(&local_velocity_flag, &total_velocity_flags, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
    comm_time += (MPI_Wtime() - t0);

    double t_comp_end = MPI_Wtime();
    double total_comp_comm_time = t_comp_end - t_comp_start;

    /* Print Summary on Rank 0 */
    if (rank == 0) {
        printf("\n--- Summary Statistics (MPI Parallel I/O np=%d) ---\n", size);
        printf("Total transactions   : %d\n", n);
        printf("Mean amount          : $%.2f\n", mean);
        printf("Std deviation        : $%.2f\n", stddev);
        printf("Amount Threshold     : $%.2f (mean + 2*stddev)\n", threshold);
        printf("High Amount Flagged  : %d\n", total_amount_flags);
        printf("Velocity Flagged (>%d): %d\n", VELOCITY_THRESHOLD, total_velocity_flags);
        printf("Total Suspicious     : %d\n", total_suspicious);
        printf("[TIMING] MPI Total (Comp+Comm) : %.6f seconds\n", total_comp_comm_time);
        printf("[TIMING] MPI Comm Portion     : %.6f seconds (%.1f%%)\n",
               comm_time, (comm_time / total_comp_comm_time) * 100.0);
        printf("[TIMING] MPI Pure Comp Portion : %.6f seconds\n", total_comp_comm_time - comm_time);
    }

    /* Cleanup */
    free(local_steps);
    free(local_amounts);
    free(local_account_ids);
    free(local_is_frauds);
    if (local_nameOrigs) {
        for (int i = 0; i < local_n; i++) free(local_nameOrigs[i]);
        free(local_nameOrigs);
    }
    free(sendcounts);
    free(displs);

    MPI_Finalize();
    return 0;
}