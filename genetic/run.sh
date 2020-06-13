#!/bin/bash

mkdir -p results
max_epochs=10
pop_size=100
chromo_size=100

p=0
pmax=3
i=0
num_exp=10

echo "Running script to compute t_seq, t_par(nw), t_ff(nw) for nw in the range [1, 2, 4, 8, .. , 128]"
echo "Every run is on an instance of generic tsp with: "$max_epochs" max epochs, "$pop_size" number of individuals, "$chromo_size" chromosome size/cities."


echo -e "\nSEQ part"
while [ "$i" -lt "$num_exp" ];
do
  ./build/seq "$max_epochs" "$pop_size" "$chromo_size" >> ./results/t_seq.data
  i=$(( i + 1 ))
done

i=0
echo "PAR part"
while [ "$i" -lt "$num_exp" ];
do
  p=0
  while [ "$p" -le "$pmax" ];
  do
    ./build/par $((2**p)) "$max_epochs" "$pop_size" "$chromo_size" >> ./results/t_par.data
    p=$(( p + 1 ))
  done
  i=$(( i + 1 ))
done

i=0

echo "FF part"
while [ "$i" -lt "$num_exp" ];
do
  p=0
  while [ "$p" -le "$pmax" ];
  do
    ./build/ff $((2**p)) "$max_epochs" "$pop_size" "$chromo_size" >> ./results/t_ff.data
    p=$(( p + 1 ))
  done
  i=$(( i + 1 ))
done
