set terminal pngcairo enhanced font "Arial,14.0" size 1080,720
set xtics font "Arial,14.0"
unset key
set output "ring-scaling.png"
set title "Measuring scalability for ring assignment (RUN 1)"
set xlabel "Number of processors"
set ylabel "Time (in seconds)"

f(x)=m*x+c
fit f(x) "final.csv" u 1:2 via m,c

plot "final.csv" u 1:2 w p pt 7 ps 2, f(x) w l lt 2 lw 3
