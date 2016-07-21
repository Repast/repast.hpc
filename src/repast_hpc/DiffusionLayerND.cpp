/*
 *   Repast for High Performance Computing (Repast HPC)
 *
 *   Copyright (c) 2010 Argonne National Laboratory
 *   All rights reserved.
 *
 *   Redistribution and use in source and binary forms, with
 *   or without modification, are permitted provided that the following
 *   conditions are met:
 *
 *  	 Redistributions of source code must retain the above copyright notice,
 *  	 this list of conditions and the following disclaimer.
 *
 *  	 Redistributions in binary form must reproduce the above copyright notice,
 *  	 this list of conditions and the following disclaimer in the documentation
 *  	 and/or other materials provided with the distribution.
 *
 *  	 Neither the name of the Argonne National Laboratory nor the names of its
 *     contributors may be used to endorse or promote products derived from
 *     this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 *   PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE TRUSTEES OR
 *   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 *   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 *   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 *   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 *   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 *   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 *   EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *
 *  DiffusionLayerND.cpp
 *
 *  Created on: July 25, 2008
 *      Author: jtm
 */
#include "DiffusionLayerND.h"


#include <boost/mpi.hpp>

using namespace std;

namespace repast {





/** UNIT TESTS **/
/*
class TestDiffusorWrapAround: public Diffusor{

  RelativeLocation relLoc;

public:
  TestDiffusorWrapAround(): relLoc(3){}


  virtual double getNewValue(double* values){
    return -1;
  }
};

class TestDiffusorStrict: public Diffusor{

  RelativeLocation relLoc;

public:
  TestDiffusorStrict(): relLoc(3){}

  virtual double getNewValue(double* values){
    return -1;
  }
};



class TestDiffusionLayerND: public ::testing::Test{

  DiffusionLayerND* diffusionLayerWrapAround;
  DiffusionLayerND* diffusionLayerStrict;

public:
  TestDiffusionLayerND(){}
  ~TestDiffusionLayerND(){}

};

TEST_F(TestDiffusionLayerND, Creation){
  repast::Point<double> origin(0, 0, 0);
  repast::Point<double> extent(100, 100, 100);
  repast::GridDimensions gd(origin, extent);
  std::vector<int> processDims;
  processDims.push_back(2);
  processDims.push_back(2);
  processDims.push_back(2);

  int rank = repast::RepastProcess::instance()->rank();

  DiffusionLayerND* diffusionLayerWrapAround = new repast::DiffusionLayerND(processDims, gd, 1, true, 0, nan("") );
  DiffusionLayerND* diffusionLayerStrict     = new repast::DiffusionLayerND(processDims, gd, 1, false, 0, nan("") );
}

TEST_F(TestDiffusionLayerND, Synchronize){

}

TEST_F(TestDiffusionLayerND, Diffusion){
  // Create a diffusor

  // Set a value at a specific location

  // Diffuse it

}

*/ // End Unit Tests

}

