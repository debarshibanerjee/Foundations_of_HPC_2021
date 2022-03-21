set terminal pngcairo enhanced font "Arial,14.0" size 1080,720
set xtics font "Arial,14.0"
set key box top left width 2 spacing 3
set output "speedup_comparison_weak.png"
set grid
set title "Speedup for MPI vs OpenMP for Weak Scalability - 1 GPU Node - Orfeo Cluster"
set xlabel "Number of MPI processes or OpenMP threads"
set ylabel "Speedup"
plot "speedup_comparison_weak.dat" u 1:2 w l lt 2 lw 3 t "OpenMP", "speedup_comparison_weak.dat" u 1:3 w l lt 3 lw 3 t "MPI", "speedup_comparison_weak.dat" u 1:1 w l lt 5 lw 3 t "Ideal"
