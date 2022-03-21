set terminal pngcairo enhanced font "Arial,14.0" size 1080,720
set xtics font "Arial,14.0"
unset key
set output "strong_mpi_scaling.png"
set grid
set title "Strong Scalability for MPI - 1 GPU Node - Orfeo Cluster"
set xlabel "Number of MPI processes"
set ylabel "Time (in seconds)"
f(x) = a*exp(-x) + b*(x**3) + c*(x**2) + d*x + e
fit f(x) 'strong_mpi_scaling.dat' u 1:2 via a,b,c,d,e
plot "strong_mpi_scaling.dat" u 1:2 w p pt 7 ps 2, f(x) w l lt 2 lw 3
