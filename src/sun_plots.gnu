set style data lines
set xrange[0:360]
set yrange[0:90]
set key off
set title "Path of Sun (from flasher location) 20140818"
set xlabel "Azimuth"
set ylabel "Zenith"
plot "overall_min_track.dat" using 4:5 lt 1 lc 1
replot "overall_max_track.dat" using 04:5 lt 1 lc 1
replot "min_track.dat" using 4:5 lt 1 lc 3
replot "max_track.dat" using 4:5 lt 1 lc 3
replot "fplane.dat" using 1:2 lt 2 lc 4
set object circle at 115,45 size 12.5 fs empty border fc rgb "black" 
set object circle at 106.95,40 size 12.5 fs empty border fc rgb "black"
set arrow from 180,0 to 180,90 nohead lt 1 lc rgb 'gray'
set arrow from 0,45 to 360,45 nohead lt 1 lc rgb 'gray'
set arrow from 0,40 to 360,40 nohead lt 1 lc rgb 'gray'
set terminal svg
set output "sunplots3.svg"
replot
set output
set terminal x11
