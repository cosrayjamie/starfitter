#!/bin/bash
for i in `cat csvs.txt`
do
    ./rmunmatched.sh $i
done