#!/usr/bin/env bash

set -eu

ROOT=$( cd $( dirname $0 )/.. ; /bin/pwd )
VERSION=$(head -n 1 $ROOT/autotools/version.txt)

HPC_NAME=repast_hpc-$VERSION
ROOT_DIST=$ROOT/out/$HPC_NAME
rm -rf $ROOT_DIST/*

DOCS=$ROOT_DIST/docs
MANUAL_INSTALL=$ROOT_DIST/MANUAL_INSTALL
SRC=$ROOT_DIST/src

mkdir -p $ROOT_DIST  $DOCS $MANUAL_INSTALL $SRC

# docs
#rm -rf $ROOT/doxygen/html $ROOT/doxygen/latex
cd $ROOT/doxygen
doxygen
cd $ROOT
mkdir -p $DOCS/api
cp -r $ROOT/doxygen/html $DOCS/api/
cp $ROOT/../docs/repast_hpc.pdf $DOCS

# SRC
rm -rf $SRC/*
mkdir -p $SRC/repast_hpc $SRC/relogo $SRC/zombie $SRC/rumor

cp $ROOT/../src/repast_hpc/*.cpp $SRC/repast_hpc
cp $ROOT/../src/repast_hpc/*.h $SRC/repast_hpc
cp $ROOT/etc/repast_hpc.module.mk $SRC/repast_hpc/module.mk

cp -r $ROOT/../src/relogo/*.cpp $SRC/relogo
cp -r $ROOT/../src/relogo/*.h $SRC/relogo
cp $ROOT/etc/relogo.module.mk $SRC/relogo/module.mk

cp -r $ROOT/../src/zombies/*.cpp $SRC/zombie
cp -r $ROOT/../src/zombies/*.h $SRC/zombie
cp $ROOT/../src/zombies/*.props $SRC/zombie
cp $ROOT/etc/zombie.module.mk $SRC/zombie/module.mk

cp -r $ROOT/../src/rumor_model/*.cpp $SRC/rumor
cp -r $ROOT/../src/rumor_model/*.h $SRC/rumor
cp $ROOT/../src/rumor_model/*.props $SRC/rumor
cp $ROOT/etc/rumor.module.mk $SRC/rumor/module.mk

# manuall install
rm -rf $MANUAL_INSTALL/*
cp $ROOT/Manual/Typical/* $MANUAL_INSTALL/

# top level
cp $ROOT/install_docs/README $ROOT/install_docs/INSTALL.txt $ROOT_DIST/
cp $ROOT/autotools/ChangeLog $ROOT_DIST/

# autotools
AT_DIR=$ROOT/autotools
cd $AT_DIR
echo "Running Autoreconf"
$( autoreconf -fi )
cd $ROOT
mkdir -p $ROOT_DIST/m4
rm -rf $ROOT_DIST/m4/*
cp -r $AT_DIR/m4 $ROOT_DIST/m4

cp $AT_DIR/src/repast_hpc/module.mk $SRC/repast_hpc
cp $AT_DIR/src/relogo/module.mk $SRC/relogo
cp $AT_DIR/src/zombie/module.mk $SRC/zombie
cp $AT_DIR/src/rumor/module.mk $SRC/rumor

REQ_FILES=("configure" "config.sub" "config.guess" "version.txt" "config.h.in" "install-sh" "Makefile.in")
for f in "${REQ_FILES[@]}"
do
  cp $AT_DIR/$f $ROOT_DIST
done

cd $ROOT/out
echo "Creating out/${HPC_NAME}.tgz"
tar cfz $HPC_NAME.tgz $HPC_NAME
cd $ROOT
