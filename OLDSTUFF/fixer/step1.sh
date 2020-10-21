#!/bin/bash
/usr/bin/find ../Mirror_Survey_Photos/ -type f -name "img_????.???.csv.matched*" | /bin/grep -v "_0_out_of" | /bin/grep -v "_1_out_of" | /usr/bin/awk -F . '{print $1"."$2"."$3"."$4"."$5}' > csvs.txt

