#!/bin/bash
# input: number of reruns, dir name where to save results

runs=$1
dir_name=$2

cd /home/lapo/Documenti/repo/parallell-programming-unipi/genetic
mkdir -p data/dir_name

echo "Running exeriments"
for((i=0;i<runs;i+=1));
do
    echo i
    cat ./scripts/configs/conf_seq | ./build/seq >> data/dir_name/seq_dell.txt
    cat ./scripts/configs/conf_par | ./build/par >> data/dir_name/seq_dell.txt
done