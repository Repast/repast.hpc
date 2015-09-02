READ ALL THESE INSTRUCTIONS BEFORE BEGINNING THE REPAST HPC INSTALL! Experienced users may find ways to customize the installation or use components that have already been installed on their machines.

The installation here assumes that none of the following libraries is installed:

MPICH2
NetCDF/NetCDF C++
Boost

It will install all of these in a local directory; it will NOT place them in /usr/lib/ or any other 'standard' location.

To avoid re-installing any of these components, comment out the appropriate sections of the install.sh script and modify the subsequent references in that script to match your existing library locations.


0. The command prompt (Terminal)

Most of what we do will be done from the command prompt or terminal. To launch this, you can go the 'Launch Pad' and find the 'Terminal' program. It may be under 'Other'.

When you have opened the Terminal, change the current directory to the RepastHPC directory, using the 'cd' command, by typing:

cd /Users/smith/Desktop/RepastHPC

1. Compiler and 'make' utility

A compiler translates the written version of the code into the instructions the computer can execute; "make" is a utility that keeps track of separate code files and knows which ones need to be compiled or re-compiled to make the final program.

Your computer may already have a compiler called 'gcc' and the 'make' utility installed. To check this, type:

gcc --version

and press 'enter'. If you get a message telling you what version of the gcc compiler you are using, then you already have gcc. If you get an error ("command not found") then you don't (and you also probably don't have 'make' either, but you can try typing 'make' in and see what happens if you like).

On a Mac: The 'clang' version of gcc can be obtained from the App store. It comes as a part of the 'XCode' package.

Go to the App Store, search for 'Xcode', and install.

After installation, you will need to:

Open XCode
Go to 'Preferences'
Select 'Downloads'
Install the 'Command Line Tools'

After this, close and re-open the terminal window, and try:

gcc --version

You should get the version information for gcc. Both gcc and 'make' are now installed.

2. Choose the folder where you would like to do the installation of Repast HPC. Note that Repast HPC includes a number of different subfolders, some of which contain libraries of other code; you should choose a directory that will be a good 'home' for all of this given the strategy you use to organize your directories and files.

3. Unzip the 'RepastHPC' compressed file. You may be able to do this by clicking on the file, but you may also need to open the Terminal window and navigate to the directory that contains the compressed file and decompressing it by typing:

tar -xvf RepastHPC.zip

4. In Terminal, navigate to the newly decompressed RepastHPC directory

5. Start the installation procedure by typing:

./install.sh

(the leading "./" are necessary)

At this point a series of installations will take place. One of the effects of this is to create a number of directories, including:

MPICH
NetCDF
Boost/Boost_1.58
RepastHPC

These directories may also be created:

mpich2-1.4.1p1
netcdf-4.2.1.1
netcdf-cxx-4.2
boost_1_58_0

If you need to re-start the installation process from scratch, simply delete all of these directories. You can also selectively delete just the ones that you need to re-install.

6. Test installation

At the end of the installation process, you should see a directory called:

repasthpc-1.0.1

To test your installation, navigate to:

repasthpc-1.0.1/MANUAL_INSTALLATION/bin

When you look in this directory, you should see the following files:

rumor_model
rumor_config.props
rumor_model.props
zombie_model
zombie_config.props
zombie_model.props

The 'rumor_model' and 'zombie_model' files are executables. To run them, you need to invoke MPI, which you do like this:

/PATH_TO_MPICH/bin/mpirun -n 8 ./rumor_model ./rumor_config.props ./rumor_model.props

or

/PATH_TO_MPICH/bin/mpirun -n 4 ./zombie_model ./zombie_config.props ./zombie_model.props

Note that the 'PATH_TO_MPICH' is the full path to the MPICH directory that was created during this installation. You may want to add this to your permanent 'PATH' variable to avoid having to type it every time.

   echo "export PATH=$MPICH_DIR/bin/:\$PATH" >>.profile
