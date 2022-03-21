set terminal pngcairo enhanced font "Arial,14.0" size 1080,720
set xtics font "Arial,14.0"
unset key
set output "mpi_speedup_strong.png"
set grid
set title "Speedup for MPI for Strong Scalability - 1 GPU Node - Orfeo Cluster"
set xlabel "Number of MPI processes"
set ylabel "Speedup"
plot "speedup_mpi_strong.dat" u 1:2 w p pt 7 ps 2, "speedup_mpi_strong.dat" u 1:2 w l lt 2 lw 3
