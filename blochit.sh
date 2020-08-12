#!/bin/bash
cd /home/thomas/starfitter/
scp bsc5.cat thomas@bloch.physics.utah.edu:starfitter/.
scp mirror_geometry.dat thomas@bloch.physics.utah.edu:starfitter/.
scp -r results/??_m??.stars.txt thomas@bloch.physics.utah.edu:starfitter/results/.
scp -r src/*.c thomas@bloch.physics.utah.edu:starfitter/src/.