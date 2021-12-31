set terminal pngcairo enhanced font "Arial,14.0" size 1080,720
set xtics font "Arial,14.0"
set output "ob1-comparison-gpu-thin.png"
set title "THIN vs GPU - ob1,tcp,ib0 - core, socket, node - bandwidth comparison"
set xlabel "Message Size (in bytes)"
set ylabel "Bandwidth (Mbytes/sec)"
set key left top Left box 3
set logscale x
p 'test.csv' u 1:2 w l lt 2 lw 3 t ' core GPU', 'test.csv' u 1:3 w l lt 3 lw 3 t ' socket GPU', 'test.csv' u 1:4 w l lt 4 lw 3 t ' node GPU', 'test.csv' u 1:5 w l lt 5 lw 3 t ' core THIN', 'test.csv' u 1:6 w l lt 6 lw 3 t ' socket THIN', 'test.csv' u 1:7 w l lt 7 lw 3 t ' node THIN'
