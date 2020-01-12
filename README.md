Analyzing Star Photographs

===============================================================================================================
Running star analysis for the first time:

If you have found this file then you have passed the first test.  There are many files in the starfitter directory so hopefully this outline will help future users get a general idea of how everything works. Please read the entire outline before running anything.

First run the command chmod +x compile.sh and then run compile.sh.  This will, as the name implies, compile all of the programs necessary to analyze star photographs.  Then run compileTAx4.sh to compile the modifications needed to analyze photos of the TAx4 telescopes.

Be sure that each telescope to be analyzed (and each time that telescope was measured) has it's own directory containing EXIF tagged photos, star measurement csv files, and a PMT measurement file.  The following are the file name formats that have to be followed:

For photos:  img_????.jpg

For star measurement files: img_????.???.csv
The first four ?s are the image number and the next three ?s are the initials of the person who measured the stars.

For PMT measurements: s?_m??_t?_p?.csv
The first ? is the site number, the nest two ?s are the mirror number, the next ? is the trial number (if a mirror was measured more than once), and the last ? is a position number (before taking photos and after taking photos).

To get star measurements and initial results be sure the telescope directories are in the Mirror_Survey_Photos/Active directory.

Once everything is compiled and files are correctly labelled and moved, run reset.sh.  Reset.sh clears prior results and creates mirror geometries via least squares optimization.  The discriptions below by Stan are very informative and should be read prior to running reset.sh.

After you have done this you will have a bunch of results in the results directory.  One should probably check process.log (in ~/starfitter) and summary.results (in ~/starfitter/results) to make sure everything worked properly.

After this you are ready to refine results and estimate error.  For the original photos you can run_boot.sh, or for TAx4 stuff you can run process_TAx4.sh.  Both programs runs boot_results.sh and /boot_stats.

boot_results.sh uses bootstrapping to find errors with /new_starfitter, which requires a current mirror_geometry.dat file (probably with the contents of summary.results) and .stars.txt files. If something goes wrong look at rejects.err to figure out what went wrong. The results are put into mirror_geometry.nparm.dat.

/boot_stats finds the median and 95% confidence interval.

Finally, in process_TAx4.sh, QuickStat.py will compile the results from different fitting parameters and compile them in the file 'TAx4_Mirror_Geometry.txt'.

-Josh Peterson

===============================================================================================================

./reset.sh will reprocess all of the star images and produce new results in the results/ subdirectory. 

It requires an initial mirror geometry file mirror_geometry.dat
It requires an up to date corner geometry file corner_geometry.dat
It requires the directory /home/thomas/Desktop/Max/Mirror_Survey_Photos/ with all of the star photos and .csv files.
It requires  /media/backup/ to be mounted so that it can rsync all of the star images, .csv files, etc. to the backup.

It runs the script ./process_results.sh 

./process_results.sh runs the program ./test_trans on the entire directory tree under /home/thomas/Desktop/Max/Mirror_Survey_Photos/

./test_trans reads the .csv files, checks the corners and matches stars. It uses exiftool to extract information from the star photos 
             and the program ./ta_stars to compute star positions from the image data. 

exiftool requires that custom EXIF tags have been added to the star images. exiftool can be used to add these tags to the images. 

Here is an example:

>exiftool Mirror_Survey_Photos/MD/20130305/img_3218.jpg | grep TA

TA Mirror Number                : 15
TA Site Name                    : Middle Drum
TA Site Number                  : 0

./ta_stars requires the bright star catalog bsc5.cat and the JPL ephemeris JPLEPH. It will connect to the InterWeb and download the 
most recent versions of the other required files, finals2000A.data and leapsec.dat.

The summary of star processing results is written to results/summary.results

If the results look good this can be concatenated to the mirror_geometry.dat file to be used as a new and improved geometry.

The script ./stars_report.sh runs the program ./new_star_report to produce the ASCII file star_report.dat

paw -b stars_report.kumac  creates star_report.hbook from stars_report.dat
paw -b starplots_all.kumac uses mirplot.kumac, star_report.hbook, and produces starplots_all.ps with plots of measured verses expected star positions.


=================================================================================================================

First you need to measure the star positions, manually, using ImageJ and create .csv files for each photograph 
containing stars. 

There are several programs needed for analyzing star photographs which are located in the /src directory. 

tranform.c reads the .csv file which was created by hand. transform first cleans up the measurements by scaling and 
rotating them to best match the known dimensions of the screen. Then it uses the program exiftool to extract 
date and time information from the .jpg photograph. Finally, it uses that information to call the program ta_stars 
and tries to match real stars with the measured star positions. Finally, it writes out the result into a .stars.txt 
file. 

Here is the example file "img_0260.csv" from the subdirectory /BR

        " " "Area"      "Mean"  "Min"   "Max"   "X"     "Y"     "XM"    "YM"    "Slice" 
1       0       58      58      58      1304.372        80.393  1304.573        80.594  0.402   "corner"
2       0       33      33      33      280.973 77.579  281.173 77.78   0.402   "corner"
3       0       12      12      12      282.58  953.457 282.781 953.658 0.402   "corner"
4       0       28      28      28      1299.95 953.056 1300.151        953.256 0.402   "corner"
0       0       0       0       0       0       0       792.1695        516.322 0       "center"
5       1096.284        134.001 6       243     880.099 550.087 880.258 550.052 0       "star"
6       504.921 91.295  0       152     751.069 320.164 751.125 319.897 0       "star"

transform.c can be compiled into three different executable versions, one for each site, 
BLACKROCK, LONGRIDGE, or MIDDLEDRUM >

>cd src
>gcc -O4 -static -D BLACKROCK -o br_trans transform.c -lm
>gcc -O4 -static -D MIDDLEDRUM -o md_trans transform.c -lm
>gcc -O4 -static -D LONGRIDGE -o lr_trans transform.c -lm
>gcc -O4 -static -o ta_stars ta_stars.c eph_manager.c nutation.c novas.c novascon.c readeph0.c solsys1.c -lm
>gcc -O4 -static -o starfitter starfitter.c poidev.c gammln.c gasdev.c nrutil.c amoeba.c amotry.c -lm
>cd ..
>ln -s src/br_trans .
>ln -s src/lr_trans .
>ln -s src/md_trans .
>ln -s src/ta_stars .
>ln -s src/starfitter .

Here is an example of how to run the transform program for the example Black Rock mirror #5 image img_260.jpg

>./br_trans 5 BR/img_0260

This should create the file BR/img_0260.stars.txt

You need to create .stars.txt files for all of the images for which you made .csv files. The .csv files must 
be in the correct format, above, or the program will not work. It looks for four corners, a center, and stars. 
Check for errors and make sure things look reasonable and that it found matching stars.

Now concatenate all of the .stars.txt files into one .stars.txt file for mirror #05

>cat BR/*.stars.txt > m5.stars.txt
>mv m5.stars.txt BR/.

Now you should have a file m5.stars.txt with lots of matched stars from different photographs and different 
times. You can use this file to fit for the geometry of the mirror.

>./starfitter BR/m5.stars.txt

Hopefully, this will print out the fitted geometry for the mirror.


  -Stan
