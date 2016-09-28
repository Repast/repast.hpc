#!/bin/bash

set -eu

# Note: Currently installs MPICH, NetCDF, NetCDF-CXX, and Boost version 1.58

BASE_DIR=$HOME/sfw
VERSION=2.2.0
REPAST_DIR=$BASE_DIR/repast_hpc-$VERSION

mkdir -p $REPAST_DIR/lib $REPAST_DIR/include

MPI_COMPILER_INVOCATION=mpicxx


# cURL
if [[ $1 == *curl* ]]
then
  if [ -e $BASE_DIR/CURL ]
  then
    echo "A directory named $BASE_DIR/CURL already exists; you must delete it before it can be rebuilt."
    exit
  fi
  if [ ! -e curl-7.42.1.tar.gz ]
  then
    echo "CURL tar file (curl-7.42.1.tar.gz) not found; you must download this and put it in this directory to continue."
    exit
  fi
  tar -xvf curl-7.42.1.tar.gz
  cd curl-7.42.1
  ./configure --prefix=$BASE_DIR/CURL
  make
  make install
  cd ..
  #ln -s $BASE_DIR/CURL/include/curl $REPAST_DIR/include/
  #ln -s $BASE_DIR/CURL/lib/libcurl.* $REPAST_DIR/lib/
fi


# MPICH
if [[ $1 == *mpich* ]]
then
  if [ -e $BASE_DIR/MPICH ]
  then
    echo "A directory named $BASE_DIR/MPICH already exists; you must delete it before it can be rebuilt."
    exit
  fi
  if [ ! -e mpich-3.1.4.tar.gz ]
  then
    echo "MPICH tar file (mpich2-1.4.1p1.tar.gz) not found; you must download this and put it in this directory to continue."
    exit
  fi
  tar -xvf mpich-3.1.4.tar.gz
  cd mpich-3.1.4
  ./configure --prefix=$BASE_DIR/MPICH --disable-fortran
  make
  make install
  cd ..
  export PATH=$BASE_DIR/MPICH/bin/:$PATH
  MPI_COMPILER_INVOCATION=$BASE_DIR/MPICH/bin/mpicxx
fi

# NETCDF

if [[ $1 == *netcdf* ]]
then
  if [ -e $BASE_DIR/NetCDF ]
  then
    echo "A directory named $BASE_DIR/NetCDF already exists; you must delete it before it can be rebuilt."
    exit
  fi
  if [ ! -e netcdf-4.2.1.1.tar.gz ]
  then
    echo "NetCDF tar file (netcdf-4.2.1.1.tar.gz) not found; you must download this and put it in this directory to continue."
    exit
  fi
  mkdir $BASE_DIR/NetCDF
  tar -xvf netcdf-4.2.1.1.tar.gz
  cd netcdf-4.2.1.1
  env CPPFLAGS=-I$BASE_DIR/CURL/include LDFLAGS=-L$BASE_DIR/CURL/lib ./configure --disable-netcdf-4 --prefix=$BASE_DIR/NetCDF
  make
  make install
  #ln -s $BASE_DIR/NetCDF/include/*.h $REPAST_DIR/include/
  #ln -s $BASE_DIR/NetCDF/lib/libnetcdf* $REPAST_DIR/lib/
  cd ..

  if [ ! -e netcdf-cxx-4.2.tar.gz ]
  then
    echo "NetCDF cpp tar file (netcdf-cxx-4.2.tar.gz) not found; you must download this and put it in this directory to continue."
    exit
  fi
  tar -xvf netcdf-cxx-4.2.tar.gz
  cd netcdf-cxx-4.2
  env CPPFLAGS=-I$BASE_DIR/NetCDF/include LDFLAGS=-L$BASE_DIR/NetCDF/lib ./configure --prefix=$BASE_DIR/NetCDF-cxx
  make
  make install
  cd ..

#ln -s $BASE_DIR/NetCDF-cxx/include/*.h $REPAST_DIR/include/
#ln -s $BASE_DIR/NetCDF-cxx/lib/libnetcdf* $REPAST_DIR/lib/
fi

# Boost



if [[ $1 == *boost* ]]
then
  if [ -e $BASE_DIR/Boost ]
  then
    echo "A directory named $BASE_DIR/Boost already exists; you must delete it before it can be rebuilt."
    exit
  fi
  if [ ! -e boost_1_61_0.tar.bz2 ]
  then
    echo "Boost tar file (boost_1_61_0.tar.bz2) not found; you must download this and put it in this directory to continue."
    exit
  fi
  echo "Extracting archive ..."
  tar -xjf boost_1_61_0.tar.bz2
  mkdir -p $BASE_DIR/Boost/Boost_1.61/include
  cp -r ./boost_1_61_0/boost $BASE_DIR/Boost/Boost_1.61/include
  cd boost_1_61_0
  ./bootstrap.sh --prefix=$BASE_DIR/Boost/Boost_1.61/ --with-libraries=system,filesystem,mpi,serialization
  echo "using mpi : $MPI_COMPILER_INVOCATION ;" >> ./project-config.jam
  ./b2 --layout=tagged variant=release threading=multi stage install
  
  if [[ $( uname -s ) == "Darwin" ]]
  then
    boost_libs=()
    for file in $BASE_DIR/Boost/Boost_1.61/lib/*.dylib
    do
      if [[ -f $file ]]; then
        boost_libs+=" $(basename $file)"
      fi
    done

    boost_libs=($boost_libs)
    for f in "${boost_libs[@]}"
    do
      for j in "${boost_libs[@]}"
      do
        if [[ $f == $j ]]; then
          $(install_name_tool -id $BASE_DIR/Boost/Boost_1.61/lib/$j $BASE_DIR/Boost/Boost_1.61/lib/$f)
        else
          $(install_name_tool -change $j $BASE_DIR/Boost/Boost_1.61/lib/$j $BASE_DIR/Boost/Boost_1.61/lib/$f)
        fi
      done
    done
  fi

  cd ..
  #ln -s $BASE_DIR/Boost/Boost_1.61/include/boost $REPAST_DIR/include/
  #ln -s $BASE_DIR/Boost/Boost_1.61/lib/libboost* $REPAST_DIR/lib/
fi

# Repast HPC
if [[ $1 == *rhpc* ]]
then
  cd ..
  CXX="$MPI_COMPILER_INVOCATION" CXXLD="$MPI_COMPILER_INVOCATION" ./configure --prefix=$REPAST_DIR --with-boost-include=$BASE_DIR/Boost/Boost_1.61/include --with-boost-lib-dir=$BASE_DIR/Boost/Boost_1.61/lib --with-boost-lib-suffix=-mt --with-netcdf-cxx=$BASE_DIR/NetCDF-cxx --with-netcdf=$BASE_DIR/NetCDF --with-curl-include=$BASE_DIR/CURL/include --with-curl-lib-dir=$BASE_DIR/CURL/lib
  # Add V=1 after "make" to see verbose compiler output
  make
  make install
  cd MANUAL_INSTALL
  # comment out "cd .." through "cd MANUAL_INSTALL" above and uncomment
  # the the line below to compile using the Makefile
  # make -f Makefile CXX=$MPI_COMPILER_INVOCATION CXXLD="$MPI_COMPILER_INVOCATION" INSTALL_DIR=$REPAST_DIR BOOST_INCLUDE_DIR=$BASE_DIR/Boost/Boost_1.61/include BOOST_LIB_DIR=$BASE_DIR/Boost/Boost_1.61/lib BOOST_INFIX=-mt NETCDF_INCLUDE_DIR=$BASE_DIR/NetCDF/include NETCDF_LIB_DIR=$BASE_DIR/NetCDF/lib NETCDF_CXX_INCLUDE_DIR=$BASE_DIR/NetCDF-cxx/include NETCDF_CXX_LIB_DIR=$BASE_DIR/NetCDF-cxx/lib CURL_INCLUDE_DIR=$BASE_DIR/CURL/include CURL_LIB_DIR=$BASE_DIR/CURL/lib
fi
