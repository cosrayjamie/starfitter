#!/bin/bash
TFILE1="./"$1".sh"
cat $1.processing_report | grep "not matched" | awk -F \( '{print $2}' | awk -F , '{print "grep -v \""$1"\" $1 | grep -v \""$2}' | awk -F \) '{print $1"\" > tmp.tmp && mv tmp.tmp $1"}'  > $TFILE1 && chmod +x $TFILE1