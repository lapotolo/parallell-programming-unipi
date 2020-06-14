# Notes (TINKERING)
TSP is the famous NP-Hard problem, summarized as:
```
“Given a list of cities and the distances between each pair of cities,
 what is the shortest possible route that visits each city and returns to the origin city?”
```

---
## GA for TSP: pseudocode and representation
```
while epoch < max_epoch
  apply_crossovers
  apply_mutations
  evaluate_popolation 
```
Let `n` be the number of chromosomes, and let `m` be the size of a single chromosome.

Then we can assume that our population is represented as `n*m` matrix. (`std::vector<std::vector<int>> population` in the code)

Together with the matrix above, we keep in memory a vector of size `n` such that its `i-th` position contains the fitness value of the `i-th` chromosome for the current generation. (`std::vector<int> chromosomes_fitness` in the code)

### Sequential model
- the evolution part is composed of two macro steps:
  - the crossover consists in an hibrydization of two chromosomes generating two brand new chromosomes. It consists of a linear scan, two-by-two, of the chromosomes in the population. Each scanned pair of chromosomes is scanned again to slice and copy part of one chromosome on to the other one and viceversa.
  - the mutation step is an `O(1)`, just a swap between two elements of the same chromosome. Since this is repeated for all the rows in total it yields an `O(n)` process.
- the population fitness evaluation is an `O(n*m)` since the two main steps are:
  - A loop to iterate over the chromosomes, giving an `O(n)`.
  - An `O(m)` step to evaluate the fitness of the single chromosome

Total time is `( O(m*n)+O(n) ) + ( O(n*m) )` 

### Parallel model (tentative)
```
while epoch < max_epoch
  apply_crossovers      |---> both these use "population" matrix (n*m)
  apply_mutations       |
  -----------------------
  evaluate_popolation   |---> this uses the vector chromosomes_fitness (1*n)
```
 we let every worker deals with one or more rows of the population matrix

 high-level representation of the computation: 
 ```
 pipe(pipe, farm)
      |_______|___ (farm_cross, farm_mutate)
              |__________ (farm_fiteval)
 ```

### FF model (plus ultra tentative)
## TSP Recasted as a genenetic algorithm
|    |                            |                  |
|----|----------------------------|------------------|
|Gene|Smallest unit of information|Location of a city|
|Chromosome|Sequence of genes| A sequence of city locations|
|Population|Collection of individual DNAs|Collection of individual routes|
|Generation|A new population of evolved DNA|A new collection of evolved routes|
|Fitness|The score or quality of a DNA| The total length of a route|
|Fitness Function| Objective function which calculates the score of a DNA| Adds up the distances between every pair of cities in the route|
|Selection|A strategy to select parent DNAs for reproduction based on their fitness|A strategy to select routes based on their fitness to create new routes|
|Crossover|A strategy to copy genes from parent DNAs to create new child DNAs| A strategy to copy locations from parent routes to build new child routes|
|Mutation|A (random) change of one or more genes in a DNA| Swapping one or more locations in a route|
