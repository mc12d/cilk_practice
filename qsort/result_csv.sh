clang++ qsort.cpp -fcilkplus -Wall -o qsort.out

echo "n_workers, time_ms" > result.csv
for n_workers in 1 2 3 4
do
	CILK_NWORKERS=${n_workers} ./qsort.out 1>> result.csv 
done
