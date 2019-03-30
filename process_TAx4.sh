#clean up
echo "Starting Cleanup"
rm mirror_geometry.nparm.dat
rm mirror_geometry.*parm.stats.csv
rm mirror_geometry.nparm.stats
echo "Cleanup finished"
#run bootstrapping and combine results
echo "Running boot_TAx4.sh"
./boot_TAx4.sh
echo "boot_TAx4 completed"
echo "Running boot_stats"
./boot_stats
echo "boot_stats completed"
echo "Compiling boot_stats results"
cat mirror_geometry.*parm.stats.csv >> mirror_geometry.nparm.stats
echo "Running QuickStat.py"
python QuickStat.py
echo "QuickStat.py completed"
