#! /bin/bash
#make clean
#make
echo "introdueix un nom per el directori\n"
read experiments
for i in 1 10 50 100 200 250 #300 500 750 999
do
    for j in 1 50 200 300 500 750 1000 2000
    do
	# i = num clients, j = num iteracions
	echo "###################################################\n"
	echo "$i clients i $j iteracions\n"
	for k in 1 2 3
	do
	    cat $experiments/prova-$k-$i-$j/launch_info | grep Time
	done
    done
done        
