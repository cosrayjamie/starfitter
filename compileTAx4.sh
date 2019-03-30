cd src
gdc -O4 -static -o test_trans transform.c julian.c -lm -D TAx4
gdc -O4 -static -o starfitter starfitter.c poidev.c gammln.c gasdev.c nrutil.c amoeba.c amotry.c -lm -D TAx4
gdc -O4 -static -o new_starfitter new_starfitter.c poidev.c gammln.c gasdev.c nrutil.c amoeba.c amotry.c -lm -D TAx4
cd ..
chmod +x boot_TAx4.sh
chmod +x process_TAx4.sh
