#!/bin/bash
#./test_trans ./Mirror_Survey_Photos/Active/ 1>>process.log 2>>process.err
#find /home/thomas/starfitterMirror_Survey_Photos/ -type f -name "img_????.???.stars.txt" -exec mv '{}' results/. \;
cat results/*img_????.???.stars.txt > results/all.stars.txt
grep -v jshhdd results/*img_????.???.stars.txt | sort -k 13 -gr > results/all.stars.csv #delete grep -v jshhdd??
find ./Mirror_Survey_Photos/Active/ -type f -name "img_????.???.csv*" -exec wc '{}' \; | awk '{print $NF" "$1}' | awk -F / '{print $NF}' | sort | uniq > results/csvfile_status.results
grep "1 1 " results/all.stars.txt > results/md_m01.stars.txt && ./starfitter results/md_m01.stars.txt | tee results/md_m01.stars.txt.results | tail -n 1 > results/summary.results
grep "1 2 " results/all.stars.txt > results/md_m02.stars.txt && ./starfitter results/md_m02.stars.txt | tee results/md_m02.stars.txt.results | tail -n 1 >> results/summary.results
grep "1 3 " results/all.stars.txt > results/md_m03.stars.txt && ./starfitter results/md_m03.stars.txt | tee results/md_m03.stars.txt.results | tail -n 1 >> results/summary.results
grep "1 4 " results/all.stars.txt > results/md_m04.stars.txt && ./starfitter results/md_m04.stars.txt | tee results/md_m04.stars.txt.results | tail -n 1 >> results/summary.results
grep "1 5 " results/all.stars.txt > results/md_m05.stars.txt && ./starfitter results/md_m05.stars.txt | tee results/md_m05.stars.txt.results | tail -n 1 >> results/summary.results
grep "1 6 " results/all.stars.txt > results/md_m06.stars.txt && ./starfitter results/md_m06.stars.txt | tee results/md_m06.stars.txt.results | tail -n 1 >> results/summary.results
grep "1 7 " results/all.stars.txt > results/md_m07.stars.txt && ./starfitter results/md_m07.stars.txt | tee results/md_m07.stars.txt.results | tail -n 1 >> results/summary.results
grep "1 8 " results/all.stars.txt > results/md_m08.stars.txt && ./starfitter results/md_m08.stars.txt | tee results/md_m08.stars.txt.results | tail -n 1 >> results/summary.results
grep "1 9 " results/all.stars.txt > results/md_m09.stars.txt && ./starfitter results/md_m09.stars.txt | tee results/md_m09.stars.txt.results | tail -n 1 >> results/summary.results
grep "1 10 " results/all.stars.txt > results/md_m10.stars.txt && ./starfitter results/md_m10.stars.txt | tee results/md_m10.stars.txt.results | tail -n 1 >> results/summary.results
grep "1 11 " results/all.stars.txt > results/md_m11.stars.txt && ./starfitter results/md_m11.stars.txt | tee results/md_m11.stars.txt.results | tail -n 1 >> results/summary.results
grep "1 12 " results/all.stars.txt > results/md_m12.stars.txt && ./starfitter results/md_m12.stars.txt | tee results/md_m12.stars.txt.results | tail -n 1 >> results/summary.results
grep "1 13 " results/all.stars.txt > results/md_m13.stars.txt && ./starfitter results/md_m13.stars.txt | tee results/md_m13.stars.txt.results | tail -n 1 >> results/summary.results
grep "1 14 " results/all.stars.txt > results/md_m14.stars.txt && ./starfitter results/md_m14.stars.txt | tee results/md_m14.stars.txt.results | tail -n 1 >> results/summary.results
grep "1 15 " results/all.stars.txt > results/md_m15.stars.txt && ./starfitter results/md_m15.stars.txt | tee results/md_m15.stars.txt.results | tail -n 1 >> results/summary.results
grep "1 16 " results/all.stars.txt > results/md_m16.stars.txt && ./starfitter results/md_m16.stars.txt | tee results/md_m16.stars.txt.results | tail -n 1 >> results/summary.results
grep "1 17 " results/all.stars.txt > results/md_m17.stars.txt && ./starfitter results/md_m17.stars.txt | tee results/md_m17.stars.txt.results | tail -n 1 >> results/summary.results
grep "1 18 " results/all.stars.txt > results/md_m18.stars.txt && ./starfitter results/md_m18.stars.txt | tee results/md_m18.stars.txt.results | tail -n 1 >> results/summary.results
grep "1 19 " results/all.stars.txt > results/md_m19.stars.txt && ./starfitter results/md_m19.stars.txt | tee results/md_m19.stars.txt.results | tail -n 1 >> results/summary.results
grep "1 20 " results/all.stars.txt > results/md_m20.stars.txt && ./starfitter results/md_m20.stars.txt | tee results/md_m20.stars.txt.results | tail -n 1 >> results/summary.results
grep "1 21 " results/all.stars.txt > results/md_m21.stars.txt && ./starfitter results/md_m21.stars.txt | tee results/md_m21.stars.txt.results | tail -n 1 >> results/summary.results
grep "1 22 " results/all.stars.txt > results/md_m22.stars.txt && ./starfitter results/md_m22.stars.txt | tee results/md_m22.stars.txt.results | tail -n 1 >> results/summary.results
grep "1 23 " results/all.stars.txt > results/md_m23.stars.txt && ./starfitter results/md_m23.stars.txt | tee results/md_m23.stars.txt.results | tail -n 1 >> results/summary.results
grep "1 24 " results/all.stars.txt > results/md_m24.stars.txt && ./starfitter results/md_m24.stars.txt | tee results/md_m24.stars.txt.results | tail -n 1 >> results/summary.results
grep "1 25 " results/all.stars.txt > results/md_m25.stars.txt && ./starfitter results/md_m25.stars.txt | tee results/md_m25.stars.txt.results | tail -n 1 >> results/summary.results
grep "1 26 " results/all.stars.txt > results/md_m26.stars.txt && ./starfitter results/md_m26.stars.txt | tee results/md_m26.stars.txt.results | tail -n 1 >> results/summary.results
grep "1 27 " results/all.stars.txt > results/md_m27.stars.txt && ./starfitter results/md_m27.stars.txt | tee results/md_m27.stars.txt.results | tail -n 1 >> results/summary.results
grep "1 28 " results/all.stars.txt > results/md_m28.stars.txt && ./starfitter results/md_m28.stars.txt | tee results/md_m28.stars.txt.results | tail -n 1 >> results/summary.results
grep "2 0 " results/all.stars.txt > results/br_m00.stars.txt && ./starfitter results/br_m00.stars.txt | tee results/br_m00.stars.txt.results | tail -n 1 >> results/summary.results
grep "2 1 " results/all.stars.txt > results/br_m01.stars.txt && ./starfitter results/br_m01.stars.txt | tee results/br_m01.stars.txt.results | tail -n 1 >> results/summary.results
grep "2 2 " results/all.stars.txt > results/br_m02.stars.txt && ./starfitter results/br_m02.stars.txt | tee results/br_m02.stars.txt.results | tail -n 1 >> results/summary.results
grep "2 3 " results/all.stars.txt > results/br_m03.stars.txt && ./starfitter results/br_m03.stars.txt | tee results/br_m03.stars.txt.results | tail -n 1 >> results/summary.results
grep "2 4 " results/all.stars.txt > results/br_m04.stars.txt && ./starfitter results/br_m04.stars.txt | tee results/br_m04.stars.txt.results | tail -n 1 >> results/summary.results
grep "2 5 " results/all.stars.txt > results/br_m05.stars.txt && ./starfitter results/br_m05.stars.txt | tee results/br_m05.stars.txt.results | tail -n 1 >> results/summary.results
grep "2 6 " results/all.stars.txt > results/br_m06.stars.txt && ./starfitter results/br_m06.stars.txt | tee results/br_m06.stars.txt.results | tail -n 1 >> results/summary.results
grep "2 7 " results/all.stars.txt > results/br_m07.stars.txt && ./starfitter results/br_m07.stars.txt | tee results/br_m07.stars.txt.results | tail -n 1 >> results/summary.results
grep "2 8 " results/all.stars.txt > results/br_m08.stars.txt && ./starfitter results/br_m08.stars.txt | tee results/br_m08.stars.txt.results | tail -n 1 >> results/summary.results
grep "2 9 " results/all.stars.txt > results/br_m09.stars.txt && ./starfitter results/br_m09.stars.txt | tee results/br_m09.stars.txt.results | tail -n 1 >> results/summary.results
grep "2 10 " results/all.stars.txt > results/br_m10.stars.txt && ./starfitter results/br_m10.stars.txt | tee results/br_m10.stars.txt.results | tail -n 1 >> results/summary.results
grep "2 11 " results/all.stars.txt > results/br_m11.stars.txt && ./starfitter results/br_m11.stars.txt | tee results/br_m11.stars.txt.results | tail -n 1 >> results/summary.results
grep "3 0 " results/all.stars.txt > results/lr_m00.stars.txt && ./starfitter results/lr_m00.stars.txt | tee results/lr_m00.stars.txt.results | tail -n 1 >> results/summary.results
grep "3 1 " results/all.stars.txt > results/lr_m01.stars.txt && ./starfitter results/lr_m01.stars.txt | tee results/lr_m01.stars.txt.results | tail -n 1 >> results/summary.results
grep "3 2 " results/all.stars.txt > results/lr_m02.stars.txt && ./starfitter results/lr_m02.stars.txt | tee results/lr_m02.stars.txt.results | tail -n 1 >> results/summary.results
grep "3 3 " results/all.stars.txt > results/lr_m03.stars.txt && ./starfitter results/lr_m03.stars.txt | tee results/lr_m03.stars.txt.results | tail -n 1 >> results/summary.results
grep "3 4 " results/all.stars.txt > results/lr_m04.stars.txt && ./starfitter results/lr_m04.stars.txt | tee results/lr_m04.stars.txt.results | tail -n 1 >> results/summary.results
grep "3 5 " results/all.stars.txt > results/lr_m05.stars.txt && ./starfitter results/lr_m05.stars.txt | tee results/lr_m05.stars.txt.results | tail -n 1 >> results/summary.results
grep "3 6 " results/all.stars.txt > results/lr_m06.stars.txt && ./starfitter results/lr_m06.stars.txt | tee results/lr_m06.stars.txt.results | tail -n 1 >> results/summary.results
grep "3 7 " results/all.stars.txt > results/lr_m07.stars.txt && ./starfitter results/lr_m07.stars.txt | tee results/lr_m07.stars.txt.results | tail -n 1 >> results/summary.results
grep "3 8 " results/all.stars.txt > results/lr_m08.stars.txt && ./starfitter results/lr_m08.stars.txt | tee results/lr_m08.stars.txt.results | tail -n 1 >> results/summary.results
grep "3 9 " results/all.stars.txt > results/lr_m09.stars.txt && ./starfitter results/lr_m09.stars.txt | tee results/lr_m09.stars.txt.results | tail -n 1 >> results/summary.results
grep "3 10 " results/all.stars.txt > results/lr_m10.stars.txt && ./starfitter results/lr_m10.stars.txt | tee results/lr_m10.stars.txt.results | tail -n 1 >> results/summary.results
grep "3 11 " results/all.stars.txt > results/lr_m11.stars.txt && ./starfitter results/lr_m11.stars.txt | tee results/lr_m11.stars.txt.results | tail -n 1 >> results/summary.results
grep "4 1 " results/all.stars.txt > results/br_TAx4_m01.stars.txt && ./starfitter results/br_TAx4_m01.stars.txt | tee results/br_TAx4_m01.stars.txt.results | tail -n 1 >> results/summary.results
grep "4 2 " results/all.stars.txt > results/br_TAx4_m02.stars.txt && ./starfitter results/br_TAx4_m02.stars.txt | tee results/br_TAx4_m02.stars.txt.results | tail -n 1 >> results/summary.results
grep "4 3 " results/all.stars.txt > results/br_TAx4_m03.stars.txt && ./starfitter results/br_TAx4_m03.stars.txt | tee results/br_TAx4_m03.stars.txt.results | tail -n 1 >> results/summary.results
grep "4 4 " results/all.stars.txt > results/br_TAx4_m04.stars.txt && ./starfitter results/br_TAx4_m04.stars.txt | tee results/br_TAx4_m04.stars.txt.results | tail -n 1 >> results/summary.results
grep "4 5 " results/all.stars.txt > results/br_TAx4_m05.stars.txt && ./starfitter results/br_TAx4_m05.stars.txt | tee results/br_TAx4_m05.stars.txt.results | tail -n 1 >> results/summary.results
grep "4 6 " results/all.stars.txt > results/br_TAx4_m06.stars.txt && ./starfitter results/br_TAx4_m06.stars.txt | tee results/br_TAx4_m06.stars.txt.results | tail -n 1 >> results/summary.results
grep "4 7 " results/all.stars.txt > results/br_TAx4_m07.stars.txt && ./starfitter results/br_TAx4_m07.stars.txt | tee results/br_TAx4_m07.stars.txt.results | tail -n 1 >> results/summary.results
grep "4 8 " results/all.stars.txt > results/br_TAx4_m08.stars.txt && ./starfitter results/br_TAx4_m08.stars.txt | tee results/br_TAx4_m08.stars.txt.results | tail -n 1 >> results/summary.results
