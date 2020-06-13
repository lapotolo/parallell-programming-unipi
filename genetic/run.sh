#!/bin/bash

mkdir -p results

# genetic algorithm parameters
# max number of iteration, number of chromosomes in the population, size of a single chromosomes.
max_epochs=10
pop_size=1000
chromo_size=100

# p is used to iterate over the powers of two that are used as increasing parallel degree
# i is used to iterate over the number of experiments
p=0
pmax=2
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
