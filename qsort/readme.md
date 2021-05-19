Implementation of parallel quicksort with cilk.

File `result.csv` contains execution time on random arrays with size 10^7 and different number of cilk workers. 

`speedup` field in `result.csv` is `time_without_cilk / time_with_cilk` 

To reproduce on your machine: 
 - Install clang++ and cilk in standard C library paths.
 - Run: `bash result_csv.sh`
