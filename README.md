# approximate-pi
A program written in C that leverages Slurm and MPI to approximate pi. 

- Class: CS4900-B90 -- HPC & Parallel Programming
- Semester: Spring 2019
- Instructor: Dr. John Nehrbass

## Usage: ./approximate [-n] [-e]
* -n: Number of points for each node to generate 
* -e: Value of epsilon 

## Input
An appropriate number of values to generate and an appropriate value for epsilon.

## Output
A brief synopsis of each nodes' execution and the approximation of pi given the provided parameters.

## Requirements
- gcc v4.8.5
- Slurm v17.11.5
- MPI v2.1.0

You may be able to execute this application with lesser versions but these are the versions of requirements the application was developed on.

## Compilation
- ./compile.sh approximate [nodes] [tasks per node]
- Add appropriate command line arguments to last line in approximate.batch
