#!/bin/bash
cat $1 | awk -F , '{print $1" "$3" "$4" "$5" "$6" "$7" "$8" "$9" "$10" "$2}' > tmp && mv tmp $1
