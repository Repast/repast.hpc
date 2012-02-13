The directory structure:

rscpp/
	bin/
		mpi_unit_tests/
		relogo_unit_tests/
		rumor/
		unit_tests/
	notes/
	src/
		relogo/
		repast_hpc/
		rumor_model/
	test/
		core/
		mpi/
		relogo/
	
	makefile


1. bin - The binaries compiled using the makefile are put in the bin directory.
These could be generated at compile time, but the output directories 
also contain some properties files and test data so its easier to have
it setup ahead of time.

2. notes - this contains some notes about the implementation. At the moment
there's only a file detailing the requirements for some template parameters.

3. src - this contains the actual source code. relogo contains the source
for relogo. repast_hpc contains the Repast HPC source. rumor_model 
contains the source for the example model described in the paper.

4. test - contains source to tests of the code in src. core contains tests
for the basic parts of repast hpc absent MPI. That is, tests for the basic
projections, etc. -- making sure if you put an Agent at 1,1 in a grid, it
actually gets put at 1,1. mpi/ contains tests of the MPI based functionality
of repast hpc -- tests for synchronizing agents, sharing agents, edges, grid
buffers etc. relogo/ contains tests for the relogo implementation. 

The makefile contains targets for compiling all of the tests and the rumor 
model. Namely,

unit_tests mpi_unit_tests relogo_unit_tests rumor_model

It also contains variable for the CXX compiler, includes etc. Set these
as appropriate.

These can be set up as makefile targets in eclipse. The actual "make" 
structure works by include specific "module.mk" files depending on the
make target. All the targets require the repast hpc source so that is included 
by default. The individual module.mk files specify the source files to 
compile and any program file or bin directory that should be used.

Running:

The unit_test binary can be run as is. The rest need to be run under
mpi -- so mpirun -np 4 X where X is the name of the executable.