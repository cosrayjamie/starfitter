Analyzing Star Photographs

===============================================================================================================
Please read before running anything.

1. Read the "TA Star Photography" wiki on the TA site: http://www.telescopearray.org/tawiki/index.php/TA_Star_Photography.
2. Follow the directions there to a) take photos on site, b) measure PMTs, and c) measure screen corners and star centroids (lovingly known as star blobs) in ImageJ. You'll need the photos for the mirror in question (~50 photos), the PMT csv file for each mirror (1 or 2), and the measured screen corner/star centroid csv files for each photo (~50) to run Starfitter.
3. Compilation. This set of commands will compile all the programs necessary to analyze star photographs.
  Navigate to the main Starfitter directory in a terminal and enter the following commands on the command line:

    $ chmod +x compile.sh #makes it an executable
    $ bash compile.sh     #runs the script using the bash shell
    $ chmod +x compileTAx4.sh
    $ bash compileTAx4.sh

4. In the main Starfitter directory, navigate to the subdirectory Mirror_Survey_Photos/Active. In this subdirectory, create a folder for each mirror trial. For example, you'll have am25_trial_1 folder, a m26_trial_2 folder, etc.

Within each of these mirror trial folders, place all the star photos for that trial (e.g., img_0001, img_0002, ... img_0047), the star/corner csv files for each photo (e.g., img_0052), and the PMT csv.
Copy  into Starfitter directory under the folder Mirror_Survey_Photos, but . Be sure that each telescope you want to analyze (including different trials of the same mirror recorded during the same night) has its own directory containing EXIF tagged photos, star measurement csv files, and a PMT measurement file.  The following are the file naming conventions that must be followed:

exiftool requires that custom EXIF tags be added to the star images. exiftool can be used to add these tags to the images.

Here is an example:

>exiftool Mirror_Survey_Photos/MD/20130305/img_3218.jpg | grep TA

TA Mirror Number                : 15
TA Site Name                    : Middle Drum
TA Site Number                  : 0

Each site has different dimensions for the cluster, dimensions used in the transformations.
Site 0: Middle Drum (Includes MD TAx4)
Site 1: Black Rock
Site 2: Long Ridge
Site 3: Black Rock TAx4

For photos: img_????.jpg
For star measurement files: img_????.???.csv

The first four ?s are the image number, and the next three ?s are the initials of the person who measured the stars.

For PMT measurements: s?_m??_t?_p?.csv

The first ? is the site number, the next two ?s are the mirror number, the next ? is the trial number (if a mirror was measured more than once during that run), and the last ? is a position number (before taking photos and after taking photos).

To get star measurements and initial results, be sure the mirror photos (just one mirror at a time) and that mirror's PMT csv file are in the Mirror_Survey_Photos/ directory.

5. Copy Mirror Data into Starfitter. Under ./starfitter/Mirror_Survey_Photos/Active, place directories for each trial of each mirror in your set. Example, from the 2018-09-20 Josh and Jamie TAx4 star photo set:
  m25 m26 m27t1 m27t2 m28t1 m28t2 m28t3

  In each mirror subdirectory, copy in the following data:
    1) The ~50 JPG photos (get rid of the extra RAW data),
    2) The ~50 CSV files of the measured corners and star blobs, and
    3) The PMT measurement file. Each trial should have its own PMT file for greater accuracy.

    NOTE: Sometimes there are two PMT files for one trial of one mirror. These are for the beginning of the photo set and the end of the photo set. In the older 2018 Josh and Jamie photo set, make two directories of the same run, each with one of the two PMT files.

    In the newer photo sets (2019 Josh and Aasutosh, 2019 Josh and Ricardo), just use the PMT measurement

6. Import Files for reset.sh. Now things are compiled and files are correctly labelled and moved. The shell script ./reset.sh will reprocess all of the star images and produce new results in the results/ subdirectory. Before it can be run, make sure you have the following and read what it does:

It requires an initial mirror geometry file, mirror_geometry.dat
It requires an up-to-date corner geometry file, corner_geometry.dat
It requires the directory ./Mirror_Survey_Photos/ with all of the star photos and .csv files.
It requires /media/backup/ to be mounted so that it can rsync all of the star images, .csv files, etc. to the backup.

It runs the script ./process_results.sh.

./process_results.sh runs the program ./transform on the entire directory tree under ./Mirror_Survey_Photos/.

./transform reads the .csv files, checks the corners and matches stars. It uses ExifTool to extract information from the star photos and the program ./ta_stars to compute star positions from the image data.

and then compile and run reset.sh. Reset.sh clears prior results and creates mirror geometries via least squares optimization.
    $ chmod +x reset.sh
    $ bash reset.sh

After you've done this, you'll have a bunch of results in the results directory. Check process.log (in ~/starfitter) and summary.results (in ~/starfitter/results) to make sure everything worked properly.

6. Refining Results and Estimating Error.  Now you can run_boot.sh, or for TAx4 photos, you can run process_TAx4.sh. Both programs run boot_results.sh and /boot_stats.

boot_results.sh uses bootstrapping to find errors with /new_starfitter, which requires a current mirror_geometry.dat file (probably with the contents of summary.results) and .stars.txt files. If something goes wrong, look at rejects.err to figure out what went wrong. The results are put into mirror_geometry.nparm.dat.

/boot_stats finds the median and 95% confidence interval.

Finally, in process_TAx4.sh, QuickStat.py will compile the results from different fitting parameters and compile them in the file TAx4_Mirror_Geometry.txt.

-Stan Thomas, Josh Peterson, Jamie Zvirzdin

===============================================================================================================





./ta_stars requires the bright star catalog bsc5.cat and the JPL ephemeris JPLEPH. It will connect to the InterWeb and download the
most recent versions of the other required files, finals2000A.data and leapsec.dat.

The summary of star processing results is written to results/summary.results

If the results look good this can be concatenated to the mirror_geometry.dat file to be used as a new and improved geometry.

The script ./stars_report.sh runs the program ./new_star_report to produce the ASCII file star_report.dat

paw -b stars_report.kumac  creates star_report.hbook from stars_report.dat
paw -b starplots_all.kumac uses mirplot.kumac, star_report.hbook, and produces starplots_all.ps with plots of measured verses expected star positions.


=================================================================================================================


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
