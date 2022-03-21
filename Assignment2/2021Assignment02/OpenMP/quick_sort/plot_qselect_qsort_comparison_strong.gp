set terminal pngcairo enhanced font "Arial,14.0" size 1080,720
set xtics font "Arial,14.0"
set key box top right width 2 spacing 3
set output "qselect_qsort_comparison_strong.png"
set grid
set title "Quick Select vs Quick Sort - Time Comparison (OpenMP) - Strong Scalability - 1 GPU Node - Orfeo Cluster"
set xlabel "Number of OpenMP threads"
set ylabel "Time (in seconds)"
plot "qselect_qsort_strong_comparison.dat" u 1:2 w l lt 2 lw 3 t "Quick Sort", "qselect_qsort_strong_comparison.dat" u 1:3 w l lt 3 lw 3 t "Quick Select"
