This is the README for Repast HPC 1.0 beta for Windows (32 bit).

-- Repast HPC has 2 prerequisites --

1. An MPI implementation. Repast HPC was compiled on windows using
Microsofts MPI implementation (MS HPC Pack), but there are others 
to choose from. 

MS HPC Pack 2008 SDK with Service Pack 2
http://www.microsoft.com/downloads/en/details.aspx?FamilyID=13644DCD-4022-4BBC-B18F-1C9E9461D5BD

mpich2:
http://www.mcs.anl.gov/research/projects/mpich2/downloads/index.php?s=downloads
Get the IA32 binaries

The remainder of this document assumes the use of the MS HPC Pack. 

2. The boost libraries must be installed. Most of the boost
libraries are header only, but a few must be compiled. Of the compiled
ones, Repast HPC requires:

	- MPI
	- Serialization (which is required by MPI)
	- Filesystem
	- System
	
Boost can be downloaded from 

http://sourceforge.net/projects/boost/files/boost/1.44.0/

Boost has its on build system and the MPI etc. libraries must be built from
scratch. Once boost has been downloaded and unpacked. Open a command prompt
and CD to wherever boost has been unpacked. Then type

bootstrap

This will build boost's "bjam" tool used to compile the actual libraries.
In order to compile, MPI we need to let boost know where our MPI implementation
is located. In order to do that, we need to modify the following file:

tools\build\v2\tools\mpi.jam

where the tools directory inside the boost directory. 

In mpi.jar replace the line (line 248) that reads:

local cluster_pack_path_native = "C:\\Program Files\\Microsoft Compute Cluster Pack" ;

with 

local cluster_pack_path_native = "C:\\Program Files\\Microsoft HPC Pack 2008 SDK" ;

Then do:

bjam --with-mpi --with-serialization --with-filesystem --with-system install

This compiles and installs into the boost headers and libraries into
C:\Boost.

For more information on compiling and using boost, see:

http://www.boost.org/doc/libs/release/more/getting_started/index.html


Using Repast HPC:

The Repast HPC distribution comes with its headers in include\ and libraries
in lib\. There are twos static libraries. 

1. repast_hpc-X.lib
2. relogo-X.lib

Where X is the version number. In both release and debug formats. The 
debug libraries have a "d" after the version info.

There are also two example applications. These can be found in
examples_bin. There are two models: 

1. Rumor: an example of rumors spreading through a network
2. Zombies: a simple relogo model where zombies infect humans
who then become zombies.

These can be run within their respective directories with:

mpiexec /np 4 rumor_model config.props rumor.props
mpiexec /np 4 zombie_model config.props zombie.props

Documentation can be found in the docs directory. 

Lastly, the distribution contains a zip archive of the Visual Studio 
2010 solution that was used to compile the libraries and example programs. 
This also includes all the source.