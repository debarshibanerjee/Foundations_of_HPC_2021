# RING:

This section can be processed using the file `run.sh` that is provided.

This script can be submitted using `qsub run.sh` and it requests for 1 thin node and 24 processors on that node to complete its task.

It uses OpenMPI-4.1.1 and GNU-9.3.0.

The following commands are used to compile and run the program -

`mpic++ ring.cc -o ring.x `

`mpirun -np 4 ring.x`

Of course the script doesn't just use 4 processors, it uses every possible number from 2 to 24 (the maximum available on a single thin node is 24).

The `plot.gp` is a `gnuplot` file is used for purposes of generating the relevant graph and least squared fitting of data. 

If you want to run it on your laptop, the `helper_scripts` directory has a `ring-laptop.sh` file which can be useful for this purpose. It should be used as `./ring-laptop.sh 4` for using 4 processors for example. It automatically detects SMT presence on Linux systems and uses `--oversubscribe` appropriately.

More details are available in the report.

# Sum 3D Matrix:

This section can be processed using the file `run.sh` that is provided.

This script can be submitted using `qsub run.sh` and it requests for 1 thin node and 24 processors on that node to complete its task.

It uses OpenMPI-4.1.1 and GNU-9.3.0.

The following commands are used to compile and run the program -

`	mpic++ sum3Dmatrix.cc -o sum3Dmatrix.x`

`mpirun -np 24 ./sum3Dmatrix.x 2400 100 100 3 4 3 2 1>matrix.dat`

Similarly other versions of the topology may be input at the command line in the format of 

`... ./exe size_1 size_2 size_3 total_dims dim_1 dim_2 dim_3`

The script reads in every possible topology in this format from the input file `matrix.in` and then executes it one by one and stores the results in `final.dat`

For running on your laptop for verification for smaller sizes, try the following -

` mpirun -np 4 ./sum3Dmatrix.x 4 4 4 3 4 1 1 1>matrix.dat`

The end of the `sum3Dmatrix.cc` file can be uncommented to print the resultant matrices for the purposes of verification.

More details are available in the report.