#!/bin/sh

# Note: Currently installs MPICH, NetCDF, NetCDF-CXX, and Boost version 1.58

MPICH_DIR=$PWD/MPICH
NETCDF_DIR=$PWD/NetCDF
BOOST_DIR=$PWD/Boost/Boost_1.58

# MPICH

if [[ ! -d "$MPICH_DIR" ]]
then
  mkdir MPICH

  tar -xvf mpich2-1.4.1p1.tar.gz
  cd mpich2-1.4.1p1
  ./configure --prefix=$MPICH_DIR --disable-f77 --disable-fc | tee c.txt
  make | tee m.txt
  make install | tee mi.txt
  cd ..
  rm -rf mpich2-1.4.1p1
fi

## Always do this
export PATH=$PWD/MPICH/bin:$PATH

# NETCDF & NETCDF C++

if [[ ! -d "$NETCDF_DIR" ]]
then
  mkdir NetCDF

  tar -xvf netcdf-4.2.1.1.tar.gz
  rm ./netcdf-4.2.1.1/oc/ocinternal.c
  cp ocinternal.c ./netcdf-4.2.1.1/oc/ocinternal.c
  cd netcdf-4.2.1.1
  ./configure --disable-netcdf-4 --prefix=$NETCDF_DIR
  make
  make install
  cd ..
  rm -rf netcdf-4.2.1.1

  tar -xvf netcdf-cxx-4.2.tar.gz
  cd netcdf-cxx-4.2
  env CPPFLAGS=-I$NETCDF_DIR/include LDFLAGS=-L$NETCDF_DIR/lib ./configure --prefix=$NETCDF_DIR
  make
  make install
  cd ..
  rm -rf netcdf-cxx-4.2
fi

# Boost

if [[ ! -d "$BOOST_DIR" ]]
then
  mkdir -p ./Boost/Boost_1.58/include
  BOOST_DIR=$PWD/Boost/Boost_1.58
  tar -xvf boost_1_58_0.tar.gz
  cp -r ./Boost_1_58_0/boost ./Boost/Boost_1.58/include
  cd boost_1_58_0
  ./bootstrap.sh --prefix=$BOOST_DIR/ --with-libraries=system,filesystem,mpi,serialization
  echo "using mpi : mpicxx ;" >>./tools/build/v2/user-config.jam
  ./b2 --layout=tagged link=static variant=release threading=multi runtime-link=static stage install
  cd ..
  rm -rf boost_1_58_0
fi

# Repast HPC

if [[ ! -d "$PWD/repasthpc-1.0.1" ]]
then
  tar -xvf repasthpc-1.0.1.tar.gz
  rm ./repasthpc-1.0.1/src/repast_hpc/DirectedVertex.h
  rm ./repasthpc-1.0.1/src/repast_hpc/UndirectedVertex.h
  cp ./*Vertex.h ./repasthpc-1.0.1/src/repast_hpc/
  cd repasthpc-1.0.1/MANUAL_INSTALLATION
  make CXX=mpicxx CXXLD=mpicxx BOOST_INCLUDE_DIR=$BOOST_DIR/include/ BOOST_LIB_DIR=$BOOST_DIR/lib/ l_BOOST="-lboost_mpi-mt-s -lboost_serialization-mt-s -lboost_system-mt-s -lboost_filesystem-mt-s" NETCDF_INCLUDE_DIR=$NETCDF_DIR/include NETCDF_LIB_DIR=$NETCDF_DIR/lib CURL_INCLUDE_DIR=/usr/include/curl CURL_LIB_DIR=/usr/lib/ 
  cd ../..
  cp -r repasthpc-1.0.1/MANUAL_INSTALLATION/lib ./Repast
  cp -r repasthpc-1.0.1/MANUAL_INSTALLATION/include ./Repast
fi



