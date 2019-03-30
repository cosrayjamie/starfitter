echo 'Starting cleanup'
rm -rf /home/jamiez/Desktop/StarStuff/starfitter/Mirror_Survey_Photos/.Trash-1000/
#rsync -avv -e ssh tamember@msg87:Desktop/Mirror_Survey_Photos /media/thomas/backup/
find /home/jamiez/Desktop/StarStuff/starfitter/Mirror_Survey_Photos/ -type f -name "img_????.???.csv.processing_report" -exec rm -f '{}' \;
find /home/jamiez/Desktop/StarStuff/starfitter/Mirror_Survey_Photos/ -type f -name "img_????.???.csv.*" | awk -F . '{print "mv -n \""$0"\" \""$1"."$2"."$3"\""}' > foo.sh
chmod +x foo.sh
./foo.sh
find /home/jamiez/Desktop/StarStuff/starfitter/Mirror_Survey_Photos/ -type f -name "*~" -exec rm -f '{}' \;
find /home/jamiez/Desktop/StarStuff/starfitter/Mirror_Survey_Photos/ -type f -name "img_????.???.csv.*" -exec rm -f '{}' \;
rm -f results/*
rm -f nohup.out
rm -f process.log
rm -f finals2000A.data
rm -f leapsec.dat
rm -f foo.sh
echo 'Cleanup finished'
#rsync -avv -e ssh tamember@msg87:Desktop/Mirror_Survey_Photos /media/thomas/backup/
#unset http_proxy
echo 'Running process_results.sh in the background'
echo 'Check process.log or results/summary.results for progress information'
nohup ./process_results.sh 1>process.log 2>process.err &

