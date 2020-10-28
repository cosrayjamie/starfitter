cp results/all.stars.csv .
sed -i 's/results\/img_//g' all.stars.csv
sed -i 's/HD//g' all.stars.csv
sed -i 's/VENUS/-2/g' all.stars.csv
sed -i 's/MARS/-4/g' all.stars.csv
sed -i 's/JUPITER/-5/g' all.stars.csv
sed -i 's/SATURN/-6/g' all.stars.csv
sed -i 's/.cnh.stars.txt:/ 1 /g' all.stars.csv
sed -i 's/.gnv.stars.txt:/ 2 /g' all.stars.csv
sed -i 's/.h_t.stars.txt:/ 3 /g' all.stars.csv
sed -i 's/.mal.stars.txt:/ 4 /g' all.stars.csv
sed -i 's/.mtc.stars.txt:/ 5 /g' all.stars.csv
sed -i 's/.nmb.stars.txt:/ 6 /g' all.stars.csv
sed -i 's/.pcc.stars.txt:/ 7 /g' all.stars.csv
sed -i 's/.sbt.stars.txt:/ 8 /g' all.stars.csv
