::===========================================================
::
:: Install.bat 
:: 
:: Will perform the following:
::
::   Install cygwin from a local package in the current
::       directory, installing gcc4, gcc4-core,gcc4-g++,
::       make,zlib, libcurl, and openssh
::
::   Install MPICH under Cygwin 
::
::   Install NetCDF under Cygwin (C and C++)
::
::   Install Boost under Cygwin
::
::   Install Repast HPC under Cygwin
::
::
:: To use, specify which component or components are to be
:: built, using the following keywords as arguments:
::
::    cygwin  mpich  netcdf  netcpp  boost repast
::
:: To specify more than one, join them with a - as:
::
:: install.bat cygwin-mpich-boost
::
::===========================================================

@setlocal enableextensions enabledelayedexpansion

set args=%1%;

if not x%args:cygwin=%==x%args% set doCygwin=TRUE

if not x%args:mpich=%==x%args% set doMPICH=TRUE

if not x%args:netcdf=%==x%args% set doNetCDF=TRUE

if not x%args:netcpp=%==x%args% set doNetCDFCPP=TRUE

if not x%args:boost=%==x%args% set doBoost=TRUE

if not x%args:repast=%==x%args% set doRepast=TRUE

:: Install cygwin

if "x%doCygwin%" EQU "xTRUE" (
  call setup.exe -q -L -l "%CD%\local-package" -R C:\cygwin -N -P gcc4,gcc4-core,gcc4-g++,make,zlib-devel,libcurl-devel,openssh >OUT.txt
  :: Make the user's directory under cygwin (and make sure you have permissions)
  mkdir C:\cygwin\home\%USERNAME%
  call C:\Windows\System32\icacls C:\cygwin\home\%USERNAME% /grant %USERNAME%:F
)


:: Copy files

copy install.sh C:\cygwin\home\%USERNAME%\install.sh

if "x%doMPICH%" EQU "xTRUE" (
  copy mpich2-1.4.1p1.tar.gz C:\cygwin\home\%USERNAME%\mpich2-1.4.1p1.tar.gz
)

if "x%doNetCDF%" EQU "xTRUE" (
  copy netcdf-4.2.1.1.tar.gz C:\cygwin\home\%USERNAME%\netcdf-4.2.1.1.tar.gz
  copy ocinternal.c C:\cygwin\home\%USERNAME%\ocinternal.c
)

if "x%doNetCDFCPP%" EQU "xTRUE" (
  copy netcdf-cxx-4.2.tar.gz C:\cygwin\home\%USERNAME%\netcdf-cxx-4.2.tar.gz
)

if "x%doBoost%" EQU "xTRUE" (
  copy boost_1_58_0.tar.gz C:\cygwin\home\%USERNAME%\boost_1_58_0.tar.gz
)

if "x%doRepast%" EQU "xTRUE" (
  copy repasthpc-1.0.1.tar.gz C:\cygwin\home\%USERNAME%\repasthpc-1.0.1.tar.gz
)

:: Go into the cygwin home directory
pushd C:\cygwin\home\%USERNAME%


:: Make subdirectories for the libraries

if "x%doMPICH%" EQU "xTRUE" (
  mkdir MPICH
)

if "x%doNetCDF%" EQU "xTRUE" (
  mkdir NetCDF
)

:: Note: cannot make NetCDFC++ without already having made NetCDF

if "x%doBoost%" EQU "xTRUE" (
  mkdir Boost
  pushd Boost
  mkdir Boost_1.58
  popd
)

if "x%doRepast%" EQU "xTRUE" (
 mkdir Repast
)

:: Next do the installation in cygwin

:: This transforms the install.sh file from DOS line termination to UNIX
call C:\cygwin\bin\dos2unix.exe install.sh

:: This enters the cygwin environment and installs all the requested software
call C:\cygwin\bin\bash --login ./install.sh %USERNAME% X%1%X

:: Cleanup
if EXIST mpich2-1.4.1p1.tar.gz del mpich2-1.4.1p1.tar.gz
if EXIST netcdf-4.2.1.1.tar.gz del netcdf-4.2.1.1.tar.gz
if EXIST netcdf-cxx-4.2.tar.gz del netcdf-cxx-4.2.tar.gz
if EXIST boost_1_58_0.tar.gz   del boost_1_58_0.tar.gz
if EXIST repast-1.0.1.tar.gz   del repast-1.0.1.tar.gz
if EXIST ocinternal.c          del ocinternal.c

del install.sh

popd
