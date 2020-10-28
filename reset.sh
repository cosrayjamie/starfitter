#!/usr/bin/env bash
echo 'Starting cleanup'
rm -rf ./Mirror_Survey_Photos/.Trash-1000/
find ./Mirror_Survey_Photos/ -type f -name "img_????.???.csv.processing_report" -exec rm -f '{}' \;
#find ./Mirror_Survey_Photos/ -type f -name "img_????.???.csv.*" | awk -F . '{print "mv -n \""$0"\" \""$1"."$2"."$3"."$4"\""}' > foo.sh
#chmod +x foo.sh
#./foo.sh
find ./Mirror_Survey_Photos/ -type f -name "*~" -exec rm -f '{}' \;
find ./Mirror_Survey_Photos/ -type f -name "img_????.???.csv.*" -exec rm -f '{}' \;
rm -f results/*
rm -f nohup.out
rm -f process.log
#rm -f finals2000A.data
#rm -f leapsec.dat
#rm -f foo.sh
echo 'Cleanup finished'
echo 'Running process_results.sh'
echo 'Check process.log or results/summary.results for progress information'
nohup ./process_results.sh 1>process.log 2>process.err &
