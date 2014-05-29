#! /bin/bash


folderName=$1
mkdir $folderName
for i in 1 10 50 100 200 250
do
    for j in 1 50 200 300 #500 750 1000 2000
    do
	# i = num clients, j = num iteracions
	for k in 1 2 3
	do
	    echo "$i clients i $j iteracions\n"
	    ./launchClient $i $j localhost 5000 $PWD/$folderName/prova-$k-$i-$j
	    python calcTimes.py \
		$PWD/$folderName/prova-$k-$i-$j \
		> $PWD/$folderName/prova-$k-$i-$j/meanTimes.txt
	    echo "$i clients i $j iteracions end!\n"
	done
    done
done  

for i in 1 10 50 100 200 250
do
    for j in 1 50 200 300 #500 750 1000 2000
    do
	
    done
done
