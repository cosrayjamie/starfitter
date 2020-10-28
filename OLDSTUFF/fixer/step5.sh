#!/bin/bash
for i in `cat csvs.txt`
do
    ./unmatched.sh $i
done