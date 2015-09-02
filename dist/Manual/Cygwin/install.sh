#!/bash.sh

# Note: Currently installs MPICH, NetCDF, NetCDF-CXX, and Boost version 1.58

# MPICH

if [[ $2 == *mpich* ]]
then
  tar -xvf mpich2-1.4.1p1.tar.gz
  cd mpich2-1.4.1p1
  ./configure --prefix=/home/$1/MPICH --disable-f77 --disable-fc |& tee c.txt
  make |& tee m.txt
  make install |& tee mi.txt
  cd ..
  echo "export PATH=/home/$1/MPICH/bin/:\$PATH" >>.profile
fi

# Always do this
export PATH=/home/$1/MPICH/bin:$PATH

# NETCDF

if [[ $2 == *netcdf* ]]
then
  tar -xvf netcdf-4.2.1.1.tar.gz
  rm ./netcdf-4.2.1.1/oc/ocinternal.c
  cp ocinternal.c ./netcdf-4.2.1.1/oc/ocinternal.c
  cd netcdf-4.2.1.1
  ./configure --disable-netcdf-4 --prefix=/home/$1/NetCDF
  make
  make install
  cd ..
fi

# NetCDF C++

if [[ $2 == *netcpp* ]]
then
  tar -xvf netcdf-cxx-4.2.tar.gz
  cd netcdf-cxx-4.2
  env CPPFLAGS=-I/home/$1/NetCDF/include LDFLAGS=-L/home/$1/NetCDF/lib ./configure --prefix=/home/$1/NetCDF
  make
  make install
  cd ..
fi

# Boost

if [[ $2 == *boost* ]]
then
  tar -xvf boost_1_58_0.tar.gz
  mkdir -p ./Boost/Boost_1.48/include
  cp -r ./Boost_1_58_0/boost ./Boost/Boost_1.48/include
  cd boost_1_58_0
  ./bootstrap.sh --prefix=/home/$1/Boost/Boost_1.58/ --with-libraries=system,filesystem,mpi,serialization
  echo "using mpi : mpicxx ;" >>./tools/build/v2/user-config.jam
  ./b2 --layout=tagged link=static variant=release threading=multi runtime-link=static stage install
  cd ..
fi

# Repast HPC

if [[ $2 == *repast* ]]
then
  tar -xvf repasthpc-1.0.1.tar.gz
  cd repasthpc-1.0.1
  cd MANUAL_INSTALLATION
  make CXX=mpicxx CXXLD=mpicxx BOOST_INCLUDE_DIR=/home/$1/Boost/Boost_1.58/include/ BOOST_LIB_DIR=/home/$1/Boost/Boost_1.58/lib/ l_BOOST="-lboost_mpi-mt-s -lboost_serialization-mt-s -lboost_system-mt-s -lboost_filesystem-mt-s" NETCDF_INCLUDE_DIR=/home/$1/NetCDF/include NETCDF_LIB_DIR=/home/$1/NetCDF/lib CURL_INCLUDE_DIR=/usr/include/curl CURL_LIB_DIR=/lib/ 
  cd ../..
  cp -r repasthpc-1.0.1/MANUAL_INSTALLATION/lib ./Repast
  cp -r repasthpc-1.0.1/MANUAL_INSTALLATION/include ./Repast
fi