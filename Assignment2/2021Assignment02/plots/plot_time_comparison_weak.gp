set terminal pngcairo enhanced font "Arial,14.0" size 1080,720
set xtics font "Arial,14.0"
set key box top left width 2 spacing 3
set output "time_comparison_weak.png"
set grid
set title "Time for MPI vs OpenMP - Weak Scalability - 1 GPU Node - Orfeo Cluster"
set xlabel "Number of MPI processes or OpenMP threads"
set ylabel "Time (in seconds)"
plot "time_comparison_weak.dat" u 1:2 w l lt 2 lw 3 t "OpenMP", "time_comparison_weak.dat" u 1:3 w l lt 3 lw 3 t "MPI"
