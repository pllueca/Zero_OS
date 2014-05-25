#! /bin/bash
for i in 1 10 50 100 200 300 500 750 1000
do
    ./clientSocket $i localhost 5000
done        

