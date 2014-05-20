#! /bin/bash
for i in 1 10 50 100 200 300 500 750 999
do
       ./launchClient $i 100 localhost 5000 p$i
done        
