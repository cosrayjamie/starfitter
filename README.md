# Analyzing Star Photographs

## Instructions

1. **Wiki.** Skim the "TA Star Photography" wiki on the TA site: http://www.telescopearray.org/tawiki/index.php/TA_Star_Photography.

2. **Take Photos.** Follow the directions there to a) take photos for each mirror in question, b) measure the first and last light-on, screen-off PMT photos, and c) measure the screen corners and star centroids (aka star blobs) in ImageJ for each light-off, screen-on photos.

3. **EXIF Tags.** The ExifTool requires that custom EXIF tags be added to the star images. The ExifTool can be used to add these tags to the images.

Here is an example:
```
$ exiftool Mirror_Survey_Photos/Active/m25_trial_1/img_0052.jpg | grep TA
```

Please regard the following file naming conventions for your photos and csv files:

Each site has different dimensions for the cluster, dimensions used in the transformations.
Site 0: Middle Drum (Includes MD TAx4)
Site 1: Black Rock
Site 2: Long Ridge
Site 3: Black Rock TAx4

TA Site Name                    : Middle Drum
TA Site Number                  : 0
TA Mirror Number                : 25

For photos: img_####.jpg
For star measurement files: img_####.AAA.csv

The first four ####s are the image number, and the next three AAAs are the initials of the person who measured the stars.

For PMT measurements: s#_m##_t#_p#.csv

The first # is the site number, the next two ##s are the mirror number, the next # is the trial number (if a mirror was measured more than once during that run), and the last # is a PMT position number (before taking photos and after taking photos).

You can get rid of the extra RAW data.

4. **Create Photo Directories.** Each "photo session" gets its own directory under ./Starfitter/Mirror_Survey_Photos/Active/ (e.g., /m25_trial_1, /m25_trial_2, /m26_trial_1, etc.), and each directory contains three kinds of files:
  - the EXIF-tagged photos from that session (~50 photos: img_0051.jpg, img_0052.jpg, ... img_0094.jpg),
  - one PMT csv file for that mirror trial (e.g., s0_m25_t1_p1.csv), and
  - the "star blob csv"--the measured screen corners and star centroids you measured (manually in ImageJ or, ideally, automated), e.g., img_0052.jaz.csv, ... img_0094.jaz.csv. One for each photo.

Each trial has its own PMT file for greater accuracy. If the starting PMT file (p1) is significantly different than the final PMT file (p2) (caused by a potential shift in the camera during the photo session), then duplicate the folder, e.g., ./Starfitter/Mirror_Survey_Photos/Active/m26_trial_1 contains the first PMT file, s0_m26_t1_p1.csv, and the duplicated /m26_trial_1_PMT2 directory contains the second PMT file, s0_m26_t1_p2.csv.

  In the newer photo sets (2019 Josh and Aasutosh, 2019 Josh and Ricardo), just use the PMT measurement

5. **Compilation.** Navigate to the main Starfitter directory in a terminal and enter the following commands on the command line:
```
    $ chmod +x compile.sh #makes it an executable
    $ bash compile.sh     #runs the script using the bash shell
    $ chmod +x compileTAx4.sh
    $ bash compileTAx4.sh
```

6. **Include Final Needed Geometry Files.** Now things are compiled and files are correctly labelled and moved. Before the shell script reset.sh can be run and process the photo sessions, make sure you have the following in ./Starfitter:

- Reset.sh requires an initial mirror geometry file: 'mirror_geometry.dat'
- It requires an up-to-date corner geometry file: 'corner_geometry.dat'
- It requires each photo session/trial to have its own directory under ./Mirror_Survey_Photos/Active/ with the files mentioned above.
- It requires /media/backup/ to be mounted so that it can rsync all of the star images, .csv files, etc. to the backup.

Now you can run ./reset.sh, which will process all of the star images and produce new results in the ./Starfitter/results/ subdirectory.
```
    $ chmod +x reset.sh
    $ bash reset.sh
```
Reset.sh clears prior results and creates mirror geometries via least squares optimization.

First, reset.sh runs the script ./process_results.sh, which runs the program ./transform on the entire directory tree under ./Mirror_Survey_Photos/.

The program ./transform reads the .csv files, checks the corners, and matches stars. It uses ExifTool to extract information from the star photos and the program ./ta_stars to compute star positions from the image data.

After reset.sh is done processing the photos, you'll have a bunch of results in the ./Starfitter/results subdirectory. Check process.log (in ~/Starfitter) and summary.results (in ~/Starfitter/results) to make sure everything worked properly.

7. **Refining Results and Estimating Error.** Now you can run_boot.sh, or for TAx4 photos, you can run process_TAx4.sh. Both programs run boot_results.sh and /boot_stats.

The shell script boot_results.sh uses bootstrapping to find errors with new_starfitter, which requires a current mirror_geometry.dat file (probably with the contents of summary.results) and .stars.txt files. If something goes wrong, look at rejects.err to figure out what went wrong. The results are put into mirror_geometry.nparm.dat.

/boot_stats finds the median and 95% confidence interval.

Finally, in process_TAx4.sh, QuickStat.py will compile the results from different fitting parameters and compile them in the file TAx4_Mirror_Geometry.txt.

-Stan Thomas, Josh Peterson, Jamie Zvirzdin







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
```
        " " "Area"      "Mean"  "Min"   "Max"   "X"     "Y"     "XM"    "YM"    "Slice"
1       0       58      58      58      1304.372        80.393  1304.573        80.594  0.402   "corner"
2       0       33      33      33      280.973 77.579  281.173 77.78   0.402   "corner"
3       0       12      12      12      282.58  953.457 282.781 953.658 0.402   "corner"
4       0       28      28      28      1299.95 953.056 1300.151        953.256 0.402   "corner"
0       0       0       0       0       0       0       792.1695        516.322 0       "center"
5       1096.284        134.001 6       243     880.099 550.087 880.258 550.052 0       "star"
6       504.921 91.295  0       152     751.069 320.164 751.125 319.897 0       "star"
```
transform.c can be compiled into three different executable versions, one for each site,
BLACKROCK, LONGRIDGE, or MIDDLEDRUM >
```
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
```
Here is an example of how to run the transform program for the example Black Rock mirror #5 image img_260.jpg
```
>./br_trans 5 BR/img_0260
```
This should create the file BR/img_0260.stars.txt

You need to create .stars.txt files for all of the images for which you made .csv files. The .csv files must
be in the correct format, above, or the program will not work. It looks for four corners, a center, and stars.
Check for errors and make sure things look reasonable and that it found matching stars.

Now concatenate all of the .stars.txt files into one .stars.txt file for mirror #05
```
>cat BR/*.stars.txt > m5.stars.txt
>mv m5.stars.txt BR/.
```
Now you should have a file m5.stars.txt with lots of matched stars from different photographs and different
times. You can use this file to fit for the geometry of the mirror.
```
>./starfitter BR/m5.stars.txt
```
Hopefully, this will print out the fitted geometry for the mirror.
