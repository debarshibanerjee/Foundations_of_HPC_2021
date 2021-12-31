set terminal pngcairo enhanced font "Arial,14.0" size 1080,720
set xtics font "Arial,14.0"
set output "ucx-comparison-gpu.png"
set title "GPU - UCX - core, socket, node - bandwidth comparison"
set xlabel "Message Size (in bytes)"
set ylabel "Bandwidth (Mbytes/sec)"
set key left top Left box 3
set logscale x
p 'test.csv' u 1:2 w l lt 2 lw 3 t ' core', 'test.csv' u 1:3 w l lt 3 lw 3 t ' socket', 'test.csv' u 1:4 w l lt 4 lw 3 t ' node'
