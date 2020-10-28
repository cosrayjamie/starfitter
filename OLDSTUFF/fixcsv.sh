#!/bin/bash
/bin/cat $1 | awk -F , '{if($1<1)$1="0"; print $1"\t"$3"\t"$4"\t"$5"\t"$6"\t"$7"\t"$8"\t"$9"\t"$10"\t"$2}' > tmp && mv tmp $1