cd src
gdc -O4 -static -o boot_stats boot_stats.c -lm
gdc -O4 -static -o test_trans transform.c julian.c -lm
#gdc -O4 -static -D_GNU_SOURCE -DSITEORIGIN -o ta_stars ta_stars.c eph_manager.c nutation.c novas.c novascon.c readeph0.c solsys1.c -lm
gdc -O4 -static -D_GNU_SOURCE -o ta_stars ta_stars.c eph_manager.c nutation.c novas.c novascon.c readeph0.c solsys1.c -lm
gdc -O4 -static -o starfitter starfitter.c poidev.c gammln.c gasdev.c nrutil.c amoeba.c amotry.c -lm
gdc -O4 -static -o new_starfitter new_starfitter.c poidev.c gammln.c gasdev.c nrutil.c amoeba.c amotry.c -lm
gdc -O4 -static -o poly_starfitter poly_starfitter.c poidev.c gammln.c gasdev.c nrutil.c amoeba.c amotry.c -lm
gdc -O4 -static -o photo_status photo_status.c -lm
gdc -O4 -static -o Mirror_Diffs Mirror_Diffs.c -lm
gdc -O4 -static -o new_star_report new_star_report.c -lm
cd ..
ln -s src/ta_stars .
ln -s src/test_trans .
ln -s src/starfitter .
ln -s src/new_starfitter .
ln -s src/poly_starfitter .
ln -s src/photo_status .
ln -s src/Mirror_Diffs .
ln -s src/boot_stats .
ln -s src/new_star_report .
chmod +x process_results.sh
chmod +x reset.sh
chmod +x fit_results.sh
chmod +x boot_results.sh
chmod +x run_boot.sh
chmod +x compileTAx4.sh
