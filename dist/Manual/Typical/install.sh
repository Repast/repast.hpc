#!/bin/bash

set -eu

# Note: Currently installs MPICH, NetCDF, NetCDF-CXX, and Boost version 1.58

BASE_DIR=$HOME/sfw
VERSION=2.3.1
REPAST_DIR=$BASE_DIR/repast_hpc-$VERSION

mkdir -p $REPAST_DIR/lib $REPAST_DIR/include

MPI_COMPILER_INVOCATION=mpicxx

CURL_VERSION="7.42.1"
CURL_DIR=curl-${CURL_VERSION}
CURL_DIST=curl-${CURL_VERSION}.tar.gz

NETCDF_VERSION="4.2.1.1"
NETCDF_DIR=netcdf-${NETCDF_VERSION}
NETCDF_DIST=netcdf-${NETCDF_VERSION}.tar.gz

NETCDFCXX_VERSION="4.2"
NETCDFCXX_DIR=netcdf-cxx-${NETCDFCXX_VERSION}
NETCDFCXX_DIST=netcdf-cxx-${NETCDFCXX_VERSION}.tar.gz

BOOST_VERSION="1_61_0"
BOOST_DIR=boost-${BOOST_VERSION}
BOOST_DIST=boost_${BOOST_VERSION}.tar.bz2


# cURL
if [[ $1 == *curl* ]]
then
  if [ -e $BASE_DIR/$CURL_DIR ]
  then
    echo "A directory named $BASE_DIR/$CURL_DIR already exists; you must delete it before it can be rebuilt."
    exit
  fi
  if [ ! -e $CURL_DIST ]
  then
    echo "CURL tar file ($CURL_DIST) not found; you must download this and put it in this directory to continue."
    exit
  fi
  tar -xvf $CURL_DIST
  cd $CURL_DIR
  ./configure --prefix=$BASE_DIR/$CURL_DIR
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
  if [ -e $BASE_DIR/$NETCDF_DIR ]
  then
    echo "A directory named $BASE_DIR/$NETCDF_DIR already exists; you must delete it before it can be rebuilt."
    exit
  fi
  if [ ! -e $NETCDF_DIST ]
  then
    echo "NetCDF tar file ($NETCDF_DIST) not found; you must download this and put it in this directory to continue."
    exit
  fi
  mkdir $BASE_DIR/$NETCDF_DIR
  tar -xvf $NETCDF_DIST
  cd $NETCDF_DIR
  env CPPFLAGS=-I$BASE_DIR/$CURL_DIR/include LDFLAGS=-L$BASE_DIR/$CURL_DIR/lib ./configure --disable-netcdf-4 --prefix=$BASE_DIR/$NETCDF_DIR
  make
  make install
  #ln -s $BASE_DIR/NetCDF/include/*.h $REPAST_DIR/include/
  #ln -s $BASE_DIR/NetCDF/lib/libnetcdf* $REPAST_DIR/lib/
  cd ..

  if [ ! -e $NETCDFCXX_DIST ]
  then
    echo "NetCDF cpp tar file ($NETCDFCXX_DIST) not found; you must download this and put it in this directory to continue."
    exit
  fi
  tar -xvf $NETCDFCXX_DIST
  cd $NETCDFCXX_DIR
  env CPPFLAGS=-I$BASE_DIR/$NETCDF_DIR/include LDFLAGS=-L$BASE_DIR/$NETCDF_DIR/lib ./configure --prefix=$BASE_DIR/$NETCDFCXX_DIR
  make
  make install
  cd ..

#ln -s $BASE_DIR/NetCDF-cxx/include/*.h $REPAST_DIR/include/
#ln -s $BASE_DIR/NetCDF-cxx/lib/libnetcdf* $REPAST_DIR/lib/
fi

# Boost



if [[ $1 == *boost* ]]
then
  if [ -e $BASE_DIR/$BOOST_DIR ]
  then
    echo "A directory named $BASE_DIR/$BOOST_DIT already exists; you must delete it before it can be rebuilt."
    exit
  fi
  if [ ! -e $BOOST_DIST ]
  then
    echo "Boost tar file ($BOOST_DIST) not found; you must download this and put it in this directory to continue."
    exit
  fi
  echo "Extracting archive ..."
  tar -xjf $BOOST_DIST
  # mkdir -p $BASE_DIR/$BOOST_DIR/include
  # cp -r ./boost_$BOOST_VERSION/boost $BASE_DIR/$BOOST_DIST/include
  cd boost_$BOOST_VERSION
  ./bootstrap.sh --prefix=$BASE_DIR/$BOOST_DIR --with-libraries=system,filesystem,mpi,serialization,chrono,timer
  echo "using mpi : $MPI_COMPILER_INVOCATION ;" >> ./project-config.jam
  ./b2 --layout=tagged variant=release threading=multi stage install
  
  if [[ $( uname -s ) == "Darwin" ]]
  then
    boost_libs=()
    for file in $BASE_DIR/$BOOST_DIR/lib/*.dylib
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
          $(install_name_tool -id $BASE_DIR/$BOOST_DIR/lib/$j $BASE_DIR/$BOOST_DIR/lib/$f)
        else
          $(install_name_tool -change $j $BASE_DIR/$BOOST_DIR/lib/$j $BASE_DIR/$BOOST_DIR/lib/$f)
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
  #cd ..
  #CXX="$MPI_COMPILER_INVOCATION" CXXLD="$MPI_COMPILER_INVOCATION" ./configure --prefix=$REPAST_DIR --#with-boost-include=$BASE_DIR/$BOOST_DIR/include --with-boost-lib-dir=$BASE_DIR/$BOOST_DIR/lib --with-#boost-lib-suffix=-mt --with-netcdf-cxx=$BASE_DIR/$NETCDFCXX_DIR --with-netcdf=$BASE_DIR/$NETCDF_DIR --#with-curl-include=$BASE_DIR/$CURL_DIR/include --with-curl-lib-dir=$BASE_DIR/$CURL_DIR/lib
  # Add V=1 after "make" to see verbose compiler output
  make
  make install
  cd MANUAL_INSTALL
  # comment out "cd .." through "cd MANUAL_INSTALL" above and uncomment
  # the the line below to compile using the Makefile
  make -f Makefile CXX=$MPI_COMPILER_INVOCATION CXXLD="$MPI_COMPILER_INVOCATION" INSTALL_DIR=$REPAST_DIR BOOST_INCLUDE_DIR=$BASE_DIR/$BOOST_DIR/include BOOST_LIB_DIR=$BASE_DIR/$BOOST_DIR/lib BOOST_INFIX=-mt NETCDF_INCLUDE_DIR=$BASE_DIR/$NETCDF_DIR/include NETCDF_LIB_DIR=$BASE_DIR/$NETCDF_DIR/lib NETCDF_CXX_INCLUDE_DIR=$BASE_DIR/$NETCDFCXX_DIR/include NETCDF_CXX_LIB_DIR=$BASE_DIR/$NETCDFCXX_DIR/lib CURL_INCLUDE_DIR=$BASE_DIR/$CURL_DIR/include CURL_LIB_DIR=$BASE_DIR/$CURL_DIR/lib
fi
