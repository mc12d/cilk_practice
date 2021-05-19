#include <cilk/cilk.h>
#include <iostream>
#include <assert.h>
#include <limits.h>
#include <time.h>


void swap(double* a, double* b) {
    double c = *a;
    *a = *b;
    *b = c;
}


// [l; r], Hoare
int _qsort_partition(double* arr, int l, int r) {
    double pivot = (arr[l] + arr[r]) / 2.0;
    while (l <= r) {
        while (arr[l] < pivot) {
            l++;
        }
        while (arr[r] > pivot) {
            r--;
        }
        if (l >= r) {
            break;
        }
        swap(arr + l, arr + r);
        l++, r--;
    }
    return r;
}


void _qsort_cilk(double* arr, int l, int r) {
    if (l >= r) {
        return;
    }
    int p = _qsort_partition(arr, l, r);
    cilk_spawn _qsort_cilk(arr, l, p);
    _qsort_cilk(arr, p + 1, r);
    cilk_sync;
}


void _qsort(double* arr, int l, int r) {
    if (l >= r) {
        return;
    }
    int p = _qsort_partition(arr, l, r);
    _qsort(arr, l, p);
    _qsort(arr, p + 1, r);
}


void qsort(double* arr, int l, int r, bool cilk = true) {
    cilk ? _qsort_cilk(arr, l, r) : _qsort(arr, l, r);
}


double* gen_random_array(int size) {
    double* arr = (double*)malloc(size * sizeof(double));

    for (int i = 0; i < size; i++) {
        arr[i] = (double)rand()/RAND_MAX * INT16_MAX;
    }
    return arr;    
}


void print_array(double* arr, int size) {
    for (int i = 0; i < size; i++) {
        printf("%lf ", arr[i]);
    }
    printf("\n");
}


void check_qsort_correctness(int arr_size) {
    double *arr = gen_random_array(arr_size);
    qsort(arr, 0, arr_size - 1);
    for (int i = 0; i < arr_size - 1; i++) {
        assert(arr[i] <= arr[i + 1] && "Array is not sorted!");
    }
    fprintf(stderr, "Qsort correctness check passed.\n");
}


int get_duration_ms(struct timespec start, struct timespec end) {
    return (1.0e+3 * end.tv_sec + 1.0e-6 * end.tv_nsec) - (1.0e+3 * start.tv_sec + 1.0e-6 * start.tv_nsec);
}


double qsort_time_ms(int arr_size, int n_stages, bool with_cilk = true) {
    
    // generate n_stages + 1 random arrays (first one is for warm-up run)
    double** arr = (double**)malloc((n_stages + 1) * sizeof(double*));
    for (int i = 0; i < n_stages + 1; i++) {
        arr[i] = gen_random_array(arr_size);
    }
    double duration_sum = 0;

    // warm-up run
    qsort(arr[0], 0, arr_size - 1);

    // n_stages rounds
    struct timespec start_ts, end_ts;
    for (int i = 1; i < n_stages + 1; i++) {
        clock_gettime(CLOCK_MONOTONIC, &start_ts);
        qsort(arr[i], 0, arr_size - 1, with_cilk);
        clock_gettime(CLOCK_MONOTONIC, &end_ts);
        duration_sum += get_duration_ms(start_ts, end_ts);
    }

    // cleanup
    for (int i = 0; i < n_stages + 1; i++) {
        free(arr[i]);
    }
    free(arr);
    return duration_sum / n_stages;
}


int main() {
    srand(time(NULL));
    
    int arr_size = 1e+7, n_stages = 10;
    int n_workers = strtol(getenv("CILK_NWORKERS"), NULL, 10);

    check_qsort_correctness(arr_size);

    double avg_time      = qsort_time_ms(arr_size, n_stages, false);
    double avg_time_cilk = qsort_time_ms(arr_size, n_stages, true);

    fprintf(stderr, "Qsort avg time: %.1lf ms\tArray size: %d\tWorkers: %d\n", avg_time_cilk, arr_size, n_workers);
    fprintf(stdout, "%d, %.1lf, %.3lf\n", n_workers, avg_time_cilk, avg_time / avg_time_cilk);
    return 0;
}
