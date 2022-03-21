set terminal pngcairo enhanced font "Arial,14.0" size 1080,720
set xtics font "Arial,14.0"
unset key
set output "omp_speedup_weak.png"
set grid
set title "Speedup for OpenMP for Weak Scalability - 1 GPU Node - Orfeo Cluster"
set xlabel "Number of OpenMP threads"
set ylabel "Speedup"
plot "speedup_omp_weak.dat" u 1:2 w p pt 7 ps 2, "speedup_omp_weak.dat" u 1:2 w l lt 2 lw 3
