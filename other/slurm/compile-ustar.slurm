#!/bin/bash

#SBATCH --job-name compile-ustar
#SBATCH --output out_compile-ustar.txt
#SBATCH --error err_compile-ustar.txt
#SBATCH --mail-user enrico.rossignolo@studenti.unipd.it
#SBATCH --mail-type ALL
#SBATCH
#SBATCH --time 60:00
#SBATCH --ntasks 1
#SBATCH --cpus-per-task 4
#SBATCH --partition allgroups
#SBATCH --mem 8G

set -e

cd "/nfsd/bcb/bcbg/rossigno/USTAR3"

srun cmake .

srun make ustar -j 4

srun make ustarx -j 4

srun make ustars -j 4
