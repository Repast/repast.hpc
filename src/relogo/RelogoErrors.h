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
 *  ReLogoErrors.h
 *
 *  Created on: Feb 22, 2012
 *      Author: JTM
 */

#ifndef RELOGO_ERRORS_H
#define RELOGO_ERRORS_H

#include <sstream>
#include <vector>
#include <stdexcept>

#include "repast_hpc/RepastErrors.h"

#include <boost/lexical_cast.hpp>

namespace repast{
namespace relogo{


/* Canonical List of ReLogo Errors */

/* ERROR 1 */
class ReLogo_Error_1: public std::invalid_argument{
public:
  ReLogo_Error_1(std::string name): INVALID_ARG(ERROR_NUMBER 1)
      THROWN_BY     "Observer::findNetwork(const std::string& name)"
      REASON        "The default directed network must be created before using it"
      EXPLANATION   "'Find Network' was called before the specified network '" + name + "' was created; "
      CAUSE         "Unknown"
      RESOLUTION    "Ensure that the network is created before any attempt to use it is made"
END_ERR


/* ERROR 2 */
class ReLogo_Error_2: public std::invalid_argument{
public:
  ReLogo_Error_2(std::string name): INVALID_ARG(ERROR_NUMBER 2)
      THROWN_BY     "Observer::findNetwork(const std::string& name)"
      REASON        "The default undirected network must be created before using it"
      EXPLANATION   "'Find Network' was called before the specified network '" + name + "' was created; "
      CAUSE         "Unknown"
      RESOLUTION    "Ensure that the network is created before any attempt to use it is made"
END_ERR


/* ERROR 3 */
class ReLogo_Error_3: public std::invalid_argument{
public:
  ReLogo_Error_3(std::string name): INVALID_ARG(ERROR_NUMBER 3)
      THROWN_BY     "Observer::findNetwork(const std::string& name)"
      REASON        "A network must be created before using it"
      EXPLANATION   "'Find Network' was called before the specified network '" + name + "' was created; " +
                    "the network was not found."
      CAUSE         "Unknown"
      RESOLUTION    "Ensure that the network is created before any attempt to use it is made"
END_ERR

/* TEMPLATE
class ReLogo_Error_: public std::invalid_argument{
public:
  ReLogo_Error_(): INVALID_ARG(ERROR_NUMBER 00)
      THROWN_BY     ""
      REASON        ""
      EXPLANATION   ""
      CAUSE         ""
      RESOLUTION    ""
END_ERR
*/

} // End ReLogo namespace
} // End Repast namespace

#endif /* RELOGO_ERRORS_H */
