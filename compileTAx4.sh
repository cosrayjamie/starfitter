#!/usr/bin/env bash
cd src
#Compile the following in the src directory.
#Create executable in the main Starfitter Directory
gcc -O4 -Wno-unused-result -static -o ../test_trans transform.c julian.c -lm -D TAx4
gcc -O4 -Wno-unused-result -static -o ../starfitter starfitter.c poidev.c gammln.c gasdev.c nrutil.c amoeba.c amotry.c -lm -D TAx4
gcc -O4 -Wno-unused-result -static -o ../new_starfitter new_starfitter.c poidev.c gammln.c gasdev.c nrutil.c amoeba.c amotry.c -lm -D TAx4
cd ..

#Make programs executable
#chmod +x boot_TAx4.sh
#chmod +x process_TAx4.sh
