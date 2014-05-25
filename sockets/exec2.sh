#! /bin/bash
#make clean
#make
echo "introdueix un nom per el directori\n"
read experiments
mkdir $experiments
for i in 1 10 50 100 200 250 #300 500 750 999
do
    for j in 1 50 200 300 500 750 1000 2000
    do
	# i = num clients, j = num iteracions
	echo "$i clients i $j iteracions\n"
	./launchClient $i $j localhost 5000 $experiments/prova-$i-$j
    done
done        
