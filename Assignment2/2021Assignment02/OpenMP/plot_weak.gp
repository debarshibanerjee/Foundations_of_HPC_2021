set terminal pngcairo enhanced font "Arial,14.0" size 1080,720
set xtics font "Arial,14.0"
unset key
set output "weak_omp_scaling.png"
set grid
set title "Weak Scalability for OpenMP - constant work per processor - 1 GPU Node - Orfeo Cluster"
set xlabel "Number of threads"
set ylabel "Time (in seconds)"
set yrange [ 0 : 23 ]
f(x) = a*(x**2) + b*x + c
fit f(x) 'weak_omp_scaling.dat' u 1:2 via a,b,c
plot "weak_omp_scaling.dat" u 1:2 w p pt 7 ps 2, f(x) w l lt 2 lw 3
