#!/bin/bash
/usr/bin/find . -type f -name "img_????.sbt.csv" -exec ./process.sh '{}' \;
/bin/mv img_????.sbt.csv /home/thomas/Desktop/Max/Mirror_Survey_Photos/MD/20130305/.
