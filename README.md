# Developer Readme for Repast HPC  #

## Requirements ##

* Boost - currently tested with 1.61.0
* netcdf-4.2.1.1
* netcdf-cxx-4.2
* curl-7.42.1
* MPI

Tar archives of these can be found in the dist/Manual directory together with some installation instructions.

## Compiling ##

1. Create a directory such as  Release or Debug.
2. Copy Makefile.tmplt into the directory and rename to Makefile
3. cd into the created directory
4. Edit the Makefile for your machine's configuration
5. Make: targets are repast_hpc, relogo, tests, clean, zombies, and rumor

Code will compile into an X/build directory where X is the directory
created in step 1. Note that the directories `Release` and `Debug` are in .gitignore in order to keep compilation 
artifacts out of git.
