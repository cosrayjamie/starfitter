#!/usr/bin/env bash

cd src
#Compile the following in the src directory.
#Create executable in the main Starfitter Directory
gcc -O4 -Wno-unused-result -static -o ../boot_stats boot_stats.c -lm
gcc -O4 -Wno-unused-result -static -o ../test_trans transform.c julian.c -lm
gcc -O4 -Wno-unused-result -static -D_GNU_SOURCE -o ../ta_stars ta_stars.c eph_manager.c nutation.c novas.c novascon.c readeph0.c solsys1.c -lm
gcc -O4 -Wno-unused-result -static -o ../starfitter starfitter.c poidev.c gammln.c gasdev.c nrutil.c amoeba.c amotry.c -lm
gcc -O4 -Wno-unused-result -static -o ../new_starfitter new_starfitter.c poidev.c gammln.c gasdev.c nrutil.c amoeba.c amotry.c -lm
gcc -O4 -Wno-unused-result -static -o ../poly_starfitter poly_starfitter.c poidev.c gammln.c gasdev.c nrutil.c amoeba.c amotry.c -lm
gcc -O4 -Wno-unused-result -static -o ../photo_status photo_status.c -lm
gcc -O4 -Wno-unused-result -static -o ../Mirror_Diffs Mirror_Diffs.c -lm
gcc -O4 -Wno-unused-result -static -o ../new_star_report new_star_report.c -lm
cd ..

#Make programs executable
#chmod +x process_results.sh
#chmod +x reset.sh
#chmod +x fit_results.sh
#chmod +x boot_results.sh
#chmod +x run_boot.sh
#chmod +x compileTAx4.sh
