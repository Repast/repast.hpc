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
 *  RepastErrors.h
 *
 *  Created on: Feb 13, 2012
 *      Author: JTM
 */

#ifndef REPAST_ERRORS_H
#define REPAST_ERRORS_H

#include <sstream>
#include <vector>
#include <stdexcept>

#include <boost/lexical_cast.hpp>


namespace repast{

std::string err_msg(int idNum, std::string thrown_by, std::string reason, std::string explanation,
    std::string cause, std::string resolution);

std::string err_msg_omit_rank(int idNum, std::string thrown_by, std::string reason, std::string explanation,
		std::string cause, std::string resolution);

template <typename T>
std::string make_vec_str(const std::vector<T>& v){
  std::stringstream ss;
  ss << "[ ";
  for(size_t i = 0; i < v.size(); i++) ss << v[i] << ", ";
  ss << "]";
  return ss.str();
}

template <typename T>
std::string make_str(const T t){
    std::stringstream ss;
    ss << t;
    return ss.str();
}


// Some syntactic sugar to make the error definitions clean
#define INVALID_ARG(_VAL) std::invalid_argument(err_msg(_VAL
#define DOMAIN_ERR(_VAL) std::domain_error(err_msg(_VAL
#define DOMAIN_ERR_OMIT_RANK(_VAL) std::domain_error(err_msg_omit_rank(_VAL
#define OUT_OF_RANGE(_VAL) std::out_of_range(err_msg(_VAL
#define ERROR_NUMBER
#define THROWN_BY   , std::string("") +
#define REASON      , std::string("") +
#define EXPLANATION , std::string("") +
#define CAUSE       , std::string("") +
#define RESOLUTION  , std::string("") +
#define END_ERR )){}};

#define VAL(a) (boost::lexical_cast<std::string>(a))


/* Canonical List of Repast HPC Errors */

/* ERROR 1 */
class Repast_Error_1: public std::domain_error{
public:
  Repast_Error_1(int val): DOMAIN_ERR(ERROR_NUMBER 1)
      THROWN_BY     "AgentImporter::decrementImportedAgentCount(int exportingRank)"
      REASON        "'exportingRank' is invalid; value = '" + VAL(val) + "'"
      EXPLANATION   "The index listing ranks that are exporting their agents to this process does not contain an entry for the specified rank."
      CAUSE         "An agent request cancellation may have been issued for an agent that was not being exported."
      RESOLUTION    "Ensure that only agents that have been requested are canceled."
END_ERR

/* ERROR 2 */
template <typename T>
class Repast_Error_2: public std::invalid_argument{
public:
  Repast_Error_2(T agId, std::string projName): INVALID_ARG(ERROR_NUMBER 2)
      THROWN_BY     "BaseGrid::moveTo(AgentId&, vector<GPType> newLocation)"
      REASON        "Agent '" + make_str(agId) + "' is not in space '" + projName + ""
      EXPLANATION   "The spatial projection that is attempting to move an agent does not contain the specified agent."
      CAUSE         "Unknown"
      RESOLUTION    "Unknown"
END_ERR

/* ERROR 3 */
class Repast_Error_3: public std::invalid_argument{
public:
  Repast_Error_3(int size, int dims): INVALID_ARG(ERROR_NUMBER 3)
      THROWN_BY     "BaseGrid::moveTo(AgentId&, vector<GPType> newLocation)"
      REASON        "'newLocation' is invalid: has " + VAL(size) + " dimensions, while space has " + VAL(dims)
      EXPLANATION   "The spatial projection is attempting to move an agent to a destination, but the destination is not fully specified;" +
                    "it has fewer dimensions than the space does."
      CAUSE         "Unknown"
      RESOLUTION    "Unknown"
END_ERR

/* ERROR 4 */
class Repast_Error_4: public std::invalid_argument{
public:
  Repast_Error_4(int numAngles, int dims): INVALID_ARG(ERROR_NUMBER 4)
      THROWN_BY     "BaseGrid::moveByVector(const T* agent, double distance, const std::vector<double>& anglesInRadians)"
      REASON        "'anglesInRadians' is invalid: has " + VAL(numAngles) + " angles, while space has " + VAL(dims) + " dimensions"
      EXPLANATION   "To move by a vector, the number of angles in the vector passed must equal the number of dimensions in the space."
      CAUSE         "Unknown"
      RESOLUTION    "Unknown"
END_ERR

/* ERROR 5 */
class Repast_Error_5: public std::invalid_argument{
public:
  Repast_Error_5(int size, int dims): INVALID_ARG(ERROR_NUMBER 5)
      THROWN_BY     "BaseGrid::moveByDisplacement(const T* agent, const std::vector<GPType>& displacement)"
      REASON        "Size of displacement vector = " + VAL(size) + " but space has " + VAL(dims) + " dimensions."
      EXPLANATION   "The vector specifying displacement must have the same number of dimensions as the space."
      CAUSE         "Unknown"
      RESOLUTION    "Unknown"
END_ERR

/* ERROR 6 */
template <typename T>
class Repast_Error_6: public std::invalid_argument{
public:
  Repast_Error_6(T id, std::string spaceName): INVALID_ARG(ERROR_NUMBER 6)
       THROWN_BY     "BaseGrid::moveByDisplacement(const T* agent, const std::vector<GPType>& displacement)"
       REASON        "Agent " + make_str(id) + " is not present in this space " + spaceName
       EXPLANATION   "The specified agent is not present in the specified space"
       CAUSE         "The agent has not yet been introduced into the space, or it has been removed."
       RESOLUTION    "Confirm that the agent being moved should be in the specified space."
END_ERR

/* ERROR 7 */
class Repast_Error_7: public std::invalid_argument{
public:
  Repast_Error_7(int P1Dim, int P2Dim): INVALID_ARG(ERROR_NUMBER 7)
       THROWN_BY     "double BaseGrid::getDistanceSq(const Point<GPType>& pt1, const Point<GPType>& pt2)"
       REASON        "Point P1 has " + VAL(P1Dim) + " dimensions while P2 has " + VAL(P2Dim) + " dimensions"
       EXPLANATION   "Points for which distance is being calculated must have the same number of dimensions"
       CAUSE         "Unknown"
       RESOLUTION    "Unknown"
END_ERR

/* ERROR 8 */
class Repast_Error_8: public std::invalid_argument{
public:
  Repast_Error_8(int P1Dim, int P2Dim): INVALID_ARG(ERROR_NUMBER 8)
       THROWN_BY     "double BaseGrid::getDisplacement(const Point<GPType>& pt1, const Point<GPType>& pt2, " +
                     "std::vector<GPType>& out))"
       REASON        "Point P1 has " + VAL(P1Dim) + " dimensions while P2 has " + VAL(P2Dim) + " dimensions"
       EXPLANATION   "Points for which displacement is being calculated must have the same number of dimensions"
       CAUSE         "Unknown"
       RESOLUTION    "Unknown"
END_ERR

/* ERROR 9 */
class Repast_Error_9: public std::invalid_argument{
public:
  Repast_Error_9(std::string projName): INVALID_ARG(ERROR_NUMBER 9)
      THROWN_BY     "Context<T>::addProjection(Projection<T>* projection)"
      REASON        "The context already contains a projection with the name '" + projName + "'"
      EXPLANATION   "Names of projections in contexts must be unique; two projections with the " +
                    "same name cannot be added, nor can the same projection be added twice."
      CAUSE         "Unknown"
      RESOLUTION    "Ensure that projections are uniquely named and only added to the context once."
END_ERR

/* ERROR 10 */
class Repast_Error_10: public std::invalid_argument{
public:
  Repast_Error_10(int dims): INVALID_ARG(ERROR_NUMBER 10)
      THROWN_BY     "Grid2DQuery<T>::Grid2DQuery(const Grid<T, int>* grid)"
      REASON        "Grid2DQuery can only be used with 2 dimensions; grid has " + VAL(dims) + " dimensions"
      EXPLANATION   "The query constructor was passed a grid that has the wrong number of dimensions."
      CAUSE         "Unknown"
      RESOLUTION    "Ensure that the grids being used with the Grid2DQuery are 2-D grids."
END_ERR

/* ERROR 11 */
template <typename T>
class Repast_Error_11: public std::out_of_range{
public:
  Repast_Error_11(std::vector<int> pt, T dimensions): OUT_OF_RANGE(ERROR_NUMBER 11)
      THROWN_BY     "void Borders::boundsCheck(const vector<int>& pt) const"
      REASON        "The point passed (at " + make_vec_str(pt) + " is outside the dimension range being checked (" + make_str(dimensions) + ")."
      EXPLANATION   "The point passed to the boundary check is outside the permitted range; it probably " +
                    "falls outside the area of the simulation being managed by this process."
      CAUSE         "Unknown"
      RESOLUTION    "Ensure that no agent is moving outside the boundaries of a process and its buffer zone."
END_ERR

/* ERROR 12 */
template <typename T>
class Repast_Error_12: public std::out_of_range{
public:
  Repast_Error_12(std::vector<int> pt, T dimensions): OUT_OF_RANGE(ERROR_NUMBER 12)
      THROWN_BY     "void Borders::boundsCheck(const vector<double>& pt) const"
      REASON        "The point passed (at " + make_vec_str(pt) + ") is outside the dimension range being checked (" + make_str(dimensions) + ")."
      EXPLANATION   "The point passed to the boundary check is outside the permitted range; it probably " +
                    "falls outside the area of the simulation being managed by this process."
      CAUSE         "Unknown"
      RESOLUTION    "Ensure that no agent is moving outside the boundaries of a process and its buffer zone."
END_ERR


/* ERROR 13 */
class Repast_Error_13: public std::invalid_argument{
public:
  Repast_Error_13(std::vector<double> oldPos, std::vector<double> newPos, std::vector<double> displacement): INVALID_ARG(ERROR_NUMBER 13)
      THROWN_BY     "StrictBorders::translate(const std::vector<double>& oldPos, std::vector<double>& newPos, const std::vector<double>& displacement)"
      REASON        "The old position (" + make_vec_str(oldPos) + "), new position (" + make_vec_str(newPos) + ") and " +
                    "displacement (" + make_vec_str(displacement) + ") must all have the same number of dimensions."
      EXPLANATION   "Sizes of old position, new position, and displacement vectors do not match " +
                    "( " + VAL(oldPos.size()) + " vs. " + VAL(newPos.size()) + " vs. " + VAL(displacement.size()) + " )."
      CAUSE         "Unknown"
      RESOLUTION    "Unknown"
END_ERR


/* ERROR 14 */
class Repast_Error_14: public std::invalid_argument{
public:
  Repast_Error_14(std::vector<int> oldPos, std::vector<int> newPos, std::vector<int> displacement): INVALID_ARG(ERROR_NUMBER 14)
      THROWN_BY     "StrictBorders::translate(const std::vector<int>& oldPos, std::vector<int>& newPos, const std::vector<int>& displacement)"
      REASON        "The old position (" + make_vec_str(oldPos) + "), new position (" + make_vec_str(newPos) + ") and " +
                    "displacement (" + make_vec_str(displacement) + ") must all have the same number of dimensions."
      EXPLANATION   "Sizes of old position, new position, and displacement vectors do not match " +
                    "( " + VAL(oldPos.size()) + " vs. " + VAL(newPos.size()) + " vs. " + VAL(displacement.size()) + " )."
      CAUSE         "Unknown"
      RESOLUTION    "Unknown"
END_ERR


/* ERROR 15 */
class Repast_Error_15: public std::invalid_argument{
public:
  Repast_Error_15(std::vector<double> oldPos, std::vector<double> newPos, std::vector<double> displacement): INVALID_ARG(ERROR_NUMBER 15)
      THROWN_BY     "WrapAroundBorders::translate(const std::vector<double>& oldPos, std::vector<double>& newPos, const std::vector<double>& displacement)"
      REASON        "The old position (" + make_vec_str(oldPos) + "), new position (" + make_vec_str(newPos) + ") and " +
                    "displacement (" + make_vec_str(displacement) + ") must all have the same number of dimensions."
      EXPLANATION   "Sizes of old position, new position, and displacement vectors do not match " +
                    "( " + VAL(oldPos.size()) + " vs. " + VAL(newPos.size()) + " vs. " + VAL(displacement.size()) + " )."
      CAUSE         "Unknown"
      RESOLUTION    "Unknown"
END_ERR


/* ERROR 16 */
class Repast_Error_16: public std::invalid_argument{
public:
  Repast_Error_16(std::vector<int> oldPos, std::vector<int> newPos, std::vector<int> displacement): INVALID_ARG(ERROR_NUMBER 16)
      THROWN_BY     "WrapAroundBorders::translate(const std::vector<int>& oldPos, std::vector<int>& newPos, const std::vector<int>& displacement)"
      REASON        "The old position (" + make_vec_str(oldPos) + "), new position (" + make_vec_str(newPos) + ") and " +
                    "displacement (" + make_vec_str(displacement) + ") must all have the same number of dimensions."
      EXPLANATION   "Sizes of old position, new position, and displacement vectors do not match " +
                    "( " + VAL(oldPos.size()) + " vs. " + VAL(newPos.size()) + " vs. " + VAL(displacement.size()) + " )."
      CAUSE         "Unknown"
      RESOLUTION    "Unknown"
END_ERR


/* ERROR 17 */
class Repast_Error_17: public std::invalid_argument{
public:
  Repast_Error_17(std::vector<double> oldPos, std::vector<double> newPos, std::vector<double> displacement): INVALID_ARG(ERROR_NUMBER 17)
      THROWN_BY     "StickyBorders::translate(const std::vector<double>& oldPos, std::vector<double>& newPos, const std::vector<double>& displacement)"
      REASON        "The old position (" + make_vec_str(oldPos) + "), new position (" + make_vec_str(newPos) + ") and " +
                    "displacement (" + make_vec_str(displacement) + ") must all have the same number of dimensions."
      EXPLANATION   "Sizes of old position, new position, and displacement vectors do not match " +
                    "( " + VAL(oldPos.size()) + " vs. " + VAL(newPos.size()) + " vs. " + VAL(displacement.size()) + " )."
      CAUSE         "Unknown"
      RESOLUTION    "Unknown"
END_ERR


/* ERROR 18 */
class Repast_Error_18: public std::invalid_argument{
public:
  Repast_Error_18(std::vector<int> oldPos, std::vector<int> newPos, std::vector<int> displacement): INVALID_ARG(ERROR_NUMBER 18)
      THROWN_BY     "StickyBorders::translate(const std::vector<int>& oldPos, std::vector<int>& newPos, const std::vector<int>& displacement)"
      REASON        "The old position (" + make_vec_str(oldPos) + "), new position (" + make_vec_str(newPos) + ") and " +
                    "displacement (" + make_vec_str(displacement) + ") must all have the same number of dimensions."
      EXPLANATION   "Sizes of old position, new position, and displacement vectors do not match " +
                    "( " + VAL(oldPos.size()) + " vs. " + VAL(newPos.size()) + " vs. " + VAL(displacement.size()) + " )."
      CAUSE         "Unknown"
      RESOLUTION    "Unknown"
END_ERR


/* ERROR 19 */
class Repast_Error_19: public std::invalid_argument{
public:
  Repast_Error_19(std::string name, std::vector<std::string> params): INVALID_ARG(ERROR_NUMBER 19)
      THROWN_BY     "createDblUni(string& name, vector<string>& params)"
      REASON        "Creating double_uniform distribution '" + name + "' requires the type of distributon " +
                    "plus two additional parameters (given " + make_vec_str(params) + ")"
      EXPLANATION   "A value in the properties specifications requested the creation of a double_uniform distribution; " +
                    "this should have been in the form\n\n:    distribution." + name + " = double_uniform,FROM,TO\n\n" +
                    "where 'FROM' is the lower boundary for the distribution and 'TO' is the upper boundary. The error occurs "+
                    "because " + VAL(params.size()) + " is too few or too many parameters."
      CAUSE         "Generally this is a problem in the properties file or in the command line arguments."
      RESOLUTION    "Alter the properties specification to provide the correct number of arguments."
END_ERR


/* ERROR 20 */
class Repast_Error_20: public std::invalid_argument{
public:
  Repast_Error_20(std::string name, std::vector<std::string> params): INVALID_ARG(ERROR_NUMBER 20)
      THROWN_BY     "createIntUni(string& name, vector<string>& params)"
      REASON        "Creating int_uniform distribution '" + name + "' requires the type of distributon " +
                    "plus two additional parameters (given " + make_vec_str(params) + ")"
      EXPLANATION   "A value in the properties specifications requested the creation of an int_uniform distribution; " +
                    "this should have been in the form\n\n:    distribution." + name + " = int_uniform,FROM,TO\n\n" +
                    "where 'FROM' is the lower boundary for the distribution and 'TO' is the upper boundary. The error occurs "+
                    "because " + VAL(params.size()) + " is too few or too many parameters."
      CAUSE         "Generally this is a problem in the properties file or in the command line arguments."
      RESOLUTION    "Alter the properties specification to provide the correct number of arguments."
END_ERR


/* ERROR 21 */
class Repast_Error_21: public std::invalid_argument{
public:
  Repast_Error_21(std::string name, std::vector<std::string> params): INVALID_ARG(ERROR_NUMBER 21)
      THROWN_BY     "createTriangle(string& name, vector<string>& params)"
      REASON        "Creating triangle distribution '" + name + "' requires the type of distributon " +
                    "plus three additional parameters (given " + make_vec_str(params) + ")"
      EXPLANATION   "A value in the properties specifications requested the creation of a triangle distribution; " +
                    "this should have been in the form\n\n:    distribution." + name + " = int_uniform,LOWER,MOST_LIKELY,UPPER\n\n" +
                    "where 'LOWER' is the lower boundary for the distribution, 'MOST_LIKELY' is the most likely value, " +
                    "and 'UPPER' is the upper boundary. The error occurs "+
                    "because " + VAL(params.size()) + " is too few or too many parameters."
      CAUSE         "Generally this is a problem in the properties file or in the command line arguments."
      RESOLUTION    "Alter the properties specification to provide the correct number of arguments."
END_ERR


/* ERROR 22 */
class Repast_Error_22: public std::invalid_argument{
public:
  Repast_Error_22(std::string name, std::vector<std::string> params): INVALID_ARG(ERROR_NUMBER 22)
      THROWN_BY     "createCauchy(string& name, vector<string>& params)"
      REASON        "Creating Cauchy distribution '" + name + "' requires the type of distributon " +
                    "plus two additional parameters (given " + make_vec_str(params) + ")"
      EXPLANATION   "A value in the properties specifications requested the creation of a Cauchy distribution; " +
                    "this should have been in the form\n\n:    distribution." + name + " = cauchy,MEDIAN,SIGMA\n\n" +
                    "where 'MEDIAN' is the median value in the distribution and 'SIGMA' is the sigma" +
                    "parameter. The error occurs because " + VAL(params.size()) + " is too few or too many parameters."
      CAUSE         "Generally this is a problem in the properties file or in the command line arguments."
      RESOLUTION    "Alter the properties specification to provide the correct number of arguments."
END_ERR


/* ERROR 23 */
class Repast_Error_23: public std::invalid_argument{
public:
  Repast_Error_23(std::string name, std::vector<std::string> params): INVALID_ARG(ERROR_NUMBER 23)
      THROWN_BY     "createExponential(string& name, vector<string>& params)"
      REASON        "Creating Exponential distribution '" + name + "' requires the type of distributon " +
                    "plus one additional parameter (given " + make_vec_str(params) + ")"
      EXPLANATION   "A value in the properties specifications requested the creation of an exponential distribution; " +
                    "this should have been in the form\n\n:    distribution." + name + " = exponential,LAMBDA\n\n" +
                    "where 'LAMBDA' is the lambda parameter. The error occurs because " +
                    VAL(params.size()) + " is too few or too many parameters."
      CAUSE         "Generally this is a problem in the properties file or in the command line arguments."
      RESOLUTION    "Alter the properties specification to provide the correct number of arguments."
END_ERR


/* ERROR 24 */
class Repast_Error_24: public std::invalid_argument{
public:
  Repast_Error_24(std::string name, std::vector<std::string> params): INVALID_ARG(ERROR_NUMBER 24)
      THROWN_BY     "createNormal(string& name, vector<string>& params)"
      REASON        "Creating Normal distribution '" + name + "' requires the type of distributon " +
                    "plus two additional parameters (given " + make_vec_str(params) + ")"
      EXPLANATION   "A value in the properties specifications requested the creation of a normal distribution; " +
                    "this should have been in the form\n\n:    distribution." + name + " = normal,MEAN,SIGMA\n\n" +
                    "where 'MEAN' is the mean value and 'SIGMA' is the sigma parameter. The error occurs because " +
                    VAL(params.size()) + " is too few or too many parameters."
      CAUSE         "Generally this is a problem in the properties file or in the command line arguments."
      RESOLUTION    "Alter the properties specification to provide the correct number of arguments."
END_ERR


/* ERROR 25 */
class Repast_Error_25: public std::invalid_argument{
public:
  Repast_Error_25(std::string name, std::vector<std::string> params): INVALID_ARG(ERROR_NUMBER 25)
      THROWN_BY     "createLogNormal(string& name, vector<string>& params)"
      REASON        "Creating Log Normal distribution '" + name + "' requires the type of distributon " +
                    "plus two additional parameters (given " + make_vec_str(params) + ")"
      EXPLANATION   "A value in the properties specifications requested the creation of a log normal distribution; " +
                    "this should have been in the form\n\n:    distribution." + name + " = lognormal,MEAN,SIGMA\n\n" +
                    "where 'MEAN' is the mean value and 'SIGMA' is the sigma parameter. The error occurs because " +
                    VAL(params.size()) + " is too few or too many parameters."
      CAUSE         "Generally this is a problem in the properties file or in the command line arguments."
      RESOLUTION    "Alter the properties specification to provide the correct number of arguments."
END_ERR


/* ERROR 26 */
class Repast_Error_26: public std::invalid_argument{
public:
  Repast_Error_26(std::string name, std::vector<std::string> params): INVALID_ARG(ERROR_NUMBER 26)
      THROWN_BY     "void initializeRandom(const Properties& props, boost::mpi::communicator* comm)"
      REASON        "Creating a distribution requires at least a distribution name (given " + make_vec_str(params) + ")"
      EXPLANATION   "A value in the properties specifications requested the creation of random distribution; " +
                    "this should have been in the form\n\n:    distribution." + name + " = TYPE,param1,param2\n\n" +
                    "where 'TYPE' is the type of distribution and param1, param2, etc. are the parameters needed " +
                    "to generate that distribution (if any). The error occurs because too few parameters are specified."
      CAUSE         "Generally this is a problem in the properties file or in the command line arguments."
      RESOLUTION    "Alter the properties specification to provide the correct number of arguments."
END_ERR


/* ERROR 27 */
class Repast_Error_27: public std::invalid_argument{
public:
  Repast_Error_27(std::string name, std::vector<std::string> params): INVALID_ARG(ERROR_NUMBER 27)
      THROWN_BY     "void initializeRandom(const Properties& props, boost::mpi::communicator* comm)"
      REASON        "Creating a distribution requires the name of a known distribution (given " + make_vec_str(params) + ")"
      EXPLANATION   "A value in the properties specifications requested the creation of random distribution; " +
                    "this should have been in the form\n\n:    distribution." + name + " = TYPE,param1,param2\n\n" +
                    "where 'TYPE' is the type of distribution and param1, param2, etc. are the parameters needed " +
                    "to generate that distribution (if any). The error occurs because the TYPE value is " +
                    "not a kind of distribution that is recognized."
      CAUSE         "Generally this is a problem in the properties file or in the command line arguments."
      RESOLUTION    "Alter the properties specification to provide the correct number of arguments."
END_ERR



/* ERROR 28 */
class Repast_Error_28: public std::domain_error{
public:
  Repast_Error_28(): DOMAIN_ERR(ERROR_NUMBER 28)
      THROWN_BY     "SVDataSet::record()"
      REASON        "Attempted to record to a data set that is not open"
      EXPLANATION   "Closed data sets cannot record"
      CAUSE         "Usually this is an error in the scheduling of data collection."
      RESOLUTION    "Ensure that no scheduled event calling 'record' occurs after the data set is closed."
END_ERR


/* ERROR 29 */
class Repast_Error_29: public std::domain_error{
public:
  Repast_Error_29(): DOMAIN_ERR(ERROR_NUMBER 29)
      THROWN_BY     "SVDataSet::write()"
      REASON        "Attempted to write a data set that is not open"
      EXPLANATION   "Closed data sets cannot write"
      CAUSE         "Usually this is an error in the scheduling of data collection."
      RESOLUTION    "Ensure that no scheduled event calling 'write' occurs after the data set is closed."
END_ERR


/* ERROR 30 */
class Repast_Error_30: public std::invalid_argument{
public:
  Repast_Error_30(int rank): INVALID_ARG(ERROR_NUMBER 30)
       THROWN_BY     "SharedNetwork<V, E>::removeSender(int rank) "
       REASON        "Sender being removed (" + VAL(rank) + ") is not in the list of senders."
       EXPLANATION   "Repast HPC is attempting to decrement the count of agents being exported " +
                     "from each rank by the SharedNetwork, but the rank specified does not have" +
                     "any value associated with it (equivalent to the rank 'sending' zero agents" +
                     "already."
       CAUSE         "Unknown"
       RESOLUTION    "Confirm that all network functions (including creation of links and moving agents) " +
                     "are being called only when agents are actually being added or removed from the " +
                     "local network."
END_ERR


/* ERROR 31 */
template <typename T>
class Repast_Error_31: public std::invalid_argument{
public:
  Repast_Error_31(T id): INVALID_ARG(ERROR_NUMBER 31)
      THROWN_BY     "SharedContext<T>::decrementProjRefCount(const AgentId& id)"
      REASON        "Id '" + make_str(id) + "' is not contained in the map of projections and references"
      EXPLANATION   "An attempt was made to remove a non-local agent from a projection, but the agent " +
                    "was not present in the map of non-local agents and their projection reference counts."
      CAUSE         "Possibly this reflects a borrowed agent being moved or cancelled more times than" +
                    "it was requested."
      RESOLUTION    "Ensure that each non-local agent is removed/cancelled only once."
END_ERR


/* ERROR 32 */
template <typename T>
class Repast_Error_32: public std::domain_error{
public:
  Repast_Error_32(T id): DOMAIN_ERR(ERROR_NUMBER 32)
      THROWN_BY     "RepastProcess::syncAgentStatus(SharedContext<T>& context, Provider& provider, AgentCreator& creator)"
      REASON        "The process has received information about an agent (" + make_str(id) + ") that does not exist on this process."
      EXPLANATION   "A process has sent information to this process that could be used to update that agent's " +
                    "status, but the agent does not exist on this process."
      CAUSE         "A problem with the balance of agent requests has occurred, or a non-local agent has been removed from " +
                    "this process without notifying the other processes that its information need no longer be sent."
      RESOLUTION    "Ensure that all changes to borrowed agents are appropriate and reconciled with their home "
                    "processes."
END_ERR


/* ERROR 33 */
class Repast_Error_33: public std::domain_error{
public:
  Repast_Error_33(): DOMAIN_ERR(ERROR_NUMBER 33)
      THROWN_BY     "SVDataSetBuilder& SVDataSetBuilder::addDataSource(SVDataSource* source)"
      REASON        "Cannot call 'addDataSource' after Data Set Builder has been used to construct a dataset"
      EXPLANATION   "The SVDataSetBuilder should be used to construct exactly on SVDataSet. The instance " +
                    "of SVDataSetBuilder should be used in a sequence: first the instance is created, then" +
                    "data sources are added (using addDataSource), then the data set is created (using " +
                    "'createDataSet'), containing all of the data sources added. This error is thrown when " +
                    "'addDataSource' is called after 'createDataSet' has already returned the Data Set being " +
                    "built."
      CAUSE         "Generally this is a problem in model initialization or scheduling of initialization events."
      RESOLUTION    "Ensure that the proper sequence of instructions is used to create the SVDataSet."
END_ERR


/* ERROR 34 */
class Repast_Error_34: public std::domain_error{
public:
  Repast_Error_34(): DOMAIN_ERR(ERROR_NUMBER 34)
      THROWN_BY     "SVDataSetBuilder::createDataSet()"
      REASON        "This function is being called for a second time, but can only be called once."
      EXPLANATION   "An instance of the SVDataSetBuilder class should be used exactly once, to create a " +
                    "single SVDataSet instance. This error occurs when the 'createDataSet' method is called " +
                    "for a second time on the same SVDataSetBuilder instance."
      CAUSE         "Generally this is a problem in model initialization or scheduling of initialization events."
      RESOLUTION    "Ensure that the proper sequence of instructions is used to create the SVDataSet."
END_ERR


/* ERROR 35 */
template <typename T>
class Repast_Error_35: public std::invalid_argument{
public:
  Repast_Error_35(T pt1, T pt2): INVALID_ARG(ERROR_NUMBER 35)
      THROWN_BY     "Point<T>::add(const Point<T> &pt)"
      REASON        "The two points being added do not have the same number of dimensions " +
                    "(" + make_str(pt1) + " vs. " + make_str(pt2) + ")"
      EXPLANATION   "Point addition can only be done if the two points have the same number of dimensions."
      CAUSE         "Unknown"
      RESOLUTION    "Ensure that all points use the same number of dimensions."
END_ERR


/* Error 36 */
template <typename T>
class Repast_Error_36: public std::invalid_argument{
public:
  Repast_Error_36(T origin, T dimensions, int originDimCount, int dimensionsDimCount): INVALID_ARG(ERROR_NUMBER 36)
      THROWN_BY     "GridDimensions::GridDimensions(Point<int> origin, Point<int> dimensions)"
      REASON        "Number of dimensions in origin (" + make_str(origin) + " = " + VAL(originDimCount) + ") " +
                    "does not equal number of dimensions in dimension specification (" + make_str(dimensions) + " = " +
                    VAL(dimensionsDimCount) + ")"
      EXPLANATION   "When creating a grid the number of dimensions used to specify the origin and the number of dimensions" +
                    "used to specify the extents must be commensurate."
      CAUSE         "Improper model initialization"
      RESOLUTION    "Reconcile the number of dimensions used."
END_ERR

/* Error 37 */
class Repast_Error_37: public std::invalid_argument{
public:
  Repast_Error_37(const std::vector<int> point, int dimensionsDimCount): INVALID_ARG(ERROR_NUMBER 37)
      THROWN_BY     "GridDimensions::contains(const std::vector<int>& pt)"
      REASON        "Number of dimensions in point (" + make_vec_str(point) + " = " + VAL(point.size()) + ") " +
                    "does not equal number of dimensions in the grid (" + VAL(dimensionsDimCount) + ")"
      EXPLANATION   "The number of dimensions in the point to be found must be the same as the number " +
                    "of dimensions in the grid."
      CAUSE         "Unknown"
      RESOLUTION    "Reconcile the number of dimensions used."
END_ERR


/* Error 38 */
class Repast_Error_38: public std::invalid_argument{
public:
  Repast_Error_38(const std::vector<double> point, int dimensionsDimCount): INVALID_ARG(ERROR_NUMBER 38)
      THROWN_BY     "GridDimensions::contains(const std::vector<double>& pt)"
      REASON        "Number of dimensions in point (" + make_vec_str(point) + " = " + VAL(point.size()) + ") " +
                    "does not equal number of dimensions in the grid (" + VAL(dimensionsDimCount) + ")"
      EXPLANATION   "The number of dimensions in the point to be found must be the same as the number " +
                    "of dimensions in the grid."
      CAUSE         "Unknown"
      RESOLUTION    "Reconcile the number of dimensions used."
END_ERR


/* Error 39 */
class Repast_Error_39: public std::domain_error{
public:
  Repast_Error_39(): DOMAIN_ERR_OMIT_RANK(ERROR_NUMBER 39)
      THROWN_BY     "RepastProcess* RepastProcess::instance()"
      REASON        "RepastProcess must be initialized before calling instance"
      EXPLANATION   "The instance of RepastProcess is not created until RepastProcess::init() is called"
      CAUSE         "Improper model initialization"
      RESOLUTION    "Call RepastProcess::init() before attempting to access the instance of RepastProcess"
END_ERR


/* Error 40 */
template <typename T>
class Repast_Error_40: public std::domain_error{
public:
  Repast_Error_40(T id, int rank, int dest1, int dest2): DOMAIN_ERR(ERROR_NUMBER 40)
      THROWN_BY     "RepastProcess::moveAgent(const AgentId& id, int process)"
      REASON        "An attempt was made to move agent " + make_str(id) + " to two different processes " +
                    "during a single iteration (from rank " + VAL(rank) + " to " + VAL(dest2) + " but already " +
                    "moved to " + VAL(dest1)
      EXPLANATION   "An agent can be moved from one process to another, but not from one process to two other " +
                    "processes; this error occurs when such an attempt is made."
      CAUSE         "Improper algorithm for agent movement."
      RESOLUTION    "Ensure than an agent moves once, then cannot be moved again."
END_ERR


/* Error 41 */
class Repast_Error_41: public std::invalid_argument{
public:
  Repast_Error_41(int max, int actual, std::string name): INVALID_ARG(ERROR_NUMBER 41)
      THROWN_BY     "ConfigLexer::ConfigLexer(const string& file_name, boost::mpi::communicator* comm, int maxConfigFileSize)"
      REASON        "The properties file '" + name + " has an actual file size of " + VAL(actual) +
                    ", which exceeds the size passed as a maximum (" + VAL(max) + ")"
      EXPLANATION   "The implementation has chosen to provide a maximum file size for the properties file, which " +
                    "is used to facilitate the transfer of the file across processes; however, the actual size of " +
                    "the file is too large, meaning that the file could only be transferred incompletely. " +
                    "Note: the actual limit is max size provided - 1"
      CAUSE         "The file size specified is too small, or the properties file too large."
      RESOLUTION    "Provide a larger file size or shrink the properties file"
END_ERR


/* Error 42 */
class Repast_Error_42: public std::invalid_argument{
public:
  Repast_Error_42(std::string configFileName): INVALID_ARG(ERROR_NUMBER 42)
      THROWN_BY     "ConfigLexer::ConfigLexer(const string& file_name, boost::mpi::communicator* comm, int maxConfigFileSize)"
      REASON        "The file '" + configFileName + "' was not found."
      EXPLANATION   "The file to be used for configuring the logger was not found."
      CAUSE         "The file name may have been specified incorrectly, or the file is not present."
      RESOLUTION    "Ensure that the file name matches an existing configuration file."
END_ERR


/* Error 43 */
class Repast_Error_43: public std::invalid_argument{
public:
  Repast_Error_43(std::string file_name): INVALID_ARG(ERROR_NUMBER 43)
      THROWN_BY     "ConfigLexer::ConfigLexer(const string& file_name, boost::mpi::communicator* comm, int maxConfigFileSize)"
      REASON        "An unknown error occurred while reading the config file '" + file_name +"'"
      EXPLANATION   "The file could not be read."
      CAUSE         "Unknown"
      RESOLUTION    "Unknown"
END_ERR

/* Error 44 */
class Repast_Error_44: public std::invalid_argument{
public:
  Repast_Error_44(std::string val): INVALID_ARG(ERROR_NUMBER 44)
      THROWN_BY     "strToUInt(const string& val)"
      REASON        "String '" + val + "' cannot be converted to unsigned int"
      EXPLANATION   "The value represented by the string cannot be converted to an unsigned integer."
      CAUSE         "Generally this is caused by improper values in properties files or command-line properties."
      RESOLUTION    "Revise the string value to represent a valid unsigned int"
END_ERR


/* Error 45 */
class Repast_Error_45: public std::invalid_argument{
public:
  Repast_Error_45(std::string val): INVALID_ARG(ERROR_NUMBER 45)
      THROWN_BY     "strToInt(const string& val)"
      REASON        "String '" + val + "' cannot be converted to int"
      EXPLANATION   "The value represented by the string cannot be converted to an integer."
      CAUSE         "Generally this is caused by improper values in properties files or command-line properties."
      RESOLUTION    "Revise the string value to represent a valid int"
END_ERR


/* Error 46 */
class Repast_Error_46: public std::invalid_argument{
public:
  Repast_Error_46(std::string val): INVALID_ARG(ERROR_NUMBER 46)
      THROWN_BY     "strToDoublet(const string& val)"
      REASON        "String '" + val + "' cannot be converted to double"
      EXPLANATION   "The value represented by the string cannot be converted to a double."
      CAUSE         "Generally this is caused by improper values in properties files or command-line properties."
      RESOLUTION    "Revise the string value to represent a valid double"
END_ERR



/* Error 47 */
template <typename T>
class Repast_Error_47: public std::out_of_range{
public:
  Repast_Error_47(int matrixDimensions, int pointDimensions, T index): OUT_OF_RANGE(ERROR_NUMBER 47)
      THROWN_BY     "Matrix<T>::boundsCheck(const Point<int>& index)"
      REASON        "Number of dimensions in index point (" + make_str(index) + " = " + VAL(pointDimensions) + ") does not equal matrix dimensions (" + std::to_string(matrixDimensions) + ")"
      EXPLANATION   "The dimensions of the index point must match those of the matrix to check boundaries"
      CAUSE         "Unknown"
      RESOLUTION    "Unknown"
END_ERR


/* Error 48 */
template <typename T>
class Repast_Error_48: public std::out_of_range{
public:
  Repast_Error_48(int dimensionIndex, int pointValue, T index, int size): OUT_OF_RANGE(ERROR_NUMBER 48)
      THROWN_BY     "Matrix<T>::boundsCheck(const Point<int>& index)"
      REASON        "Value of coordinate " + VAL(dimensionIndex) + " in point " + make_str(index) + " is " + VAL(pointValue) + " which is outside range 0 - " + VAL(size)
      EXPLANATION   "The point must specify a valid entry in the matrix; the values that make up the point must " +
                    "be greater than zero and less than the size of the matrix on the given dimension"
      CAUSE         "Unknown"
      RESOLUTION    "Unknown"
END_ERR


/* Error 49 */
template <typename T>
class Repast_Error_49: public std::invalid_argument{
public:
  Repast_Error_49(size_t dims, T gridDimensions): INVALID_ARG(ERROR_NUMBER 49)
      THROWN_BY     "SharedBaseGrid<T, GPTransformer, Adder, GPType>::SharedBaseGrid(std::string name, " +
                    "GridDimensions gridDims, std::vector<int> processDims, int buffer, boost::mpi::communicator* comm)"
      REASON        "Invalid number of grid dimensions (" + VAL (dims) + "): " + make_str(gridDimensions)
      EXPLANATION   "SharedBaseGrid can only use 1- or 2-dimensional grids"
      CAUSE         "Improper model construction"
      RESOLUTION    "Modify the grid to be only 1 or 2 dimensions"
END_ERR


/* Error 50 */
template <typename T>
class Repast_Error_50: public std::invalid_argument{
public:
  Repast_Error_50(size_t dims, T gridDimensions, int processDims): INVALID_ARG(ERROR_NUMBER 50)
      THROWN_BY     "SharedBaseGrid<T, GPTransformer, Adder, GPType>::SharedBaseGrid(std::string name, " +
                    "GridDimensions gridDims, std::vector<int> processDims, int buffer, boost::mpi::communicator* comm)"
      REASON        "Invalid number of grid dimensions (" + VAL (dims) + "): " + make_str(gridDimensions) + " "
                    "Does not match number of process dimensions (" + VAL(processDims) + ")"
      EXPLANATION   "SharedBaseGrid dimensions must match number of process dimensions"
      CAUSE         "Improper model construction, or improper process topology"
      RESOLUTION    "Modify the grid or the process toplogy to match"
END_ERR


/* Error 51 */
class Repast_Error_51: public std::invalid_argument{
public:
  Repast_Error_51(int dimCount, int extent, double pCount): INVALID_ARG(ERROR_NUMBER 51)
      THROWN_BY     "SharedBaseGrid<T, GPTransformer, Adder, GPType>::SharedBaseGrid(std::string name, " +
                    "GridDimensions gridDims, std::vector<int> processDims, int buffer, boost::mpi::communicator* comm)"
      REASON        "Number of processes in a given dimension must divide evenly into the extent of that dimension; " +
                    "In dimension " + VAL(dimCount) + " the extent is " + VAL(extent) + " but the process count is " + VAL(pCount)
      EXPLANATION   "Repast HPC will try to apportion the total simulation grid among some set of processes; to " +
                    "achieve this, the grid dimensions of the space must divide evenly among the processes. So, if " +
                    "the total space is 900 in the x dimension and 900 in the y dimension, and there are 9 processes " +
                    "in a 3 x 3 grid, this will work, with 300 x 300 on each process. However, if the space is 1000 x 1000 " +
                    " a 3 x 3 grid cannot be used."
      CAUSE         "The extent of space and the number of processes in the x and y directions, as specified " +
                    "in the properties file, are not compatible."
      RESOLUTION    "Modify the model parameters to make the space evenly divisible by the processes in each "
                    "dimension."
END_ERR



/* Error 52 */
class Repast_Error_52: public std::invalid_argument{
public:
  Repast_Error_52(int maxFileSize, int actualSize, std::string name): INVALID_ARG(ERROR_NUMBER 52)
      THROWN_BY     "Properties::readFile(const std::string& file, boost::mpi::communicator* comm, int maxPropFileSize)"
      REASON        "Actual file size (" + VAL(actualSize) + ") exceeds specified maximum file size (" + VAL(maxFileSize) + " for Properties file '" + name + "'"
      EXPLANATION   "A maximum expected file size can be specified to make the sharing of the properties file " +
                    "among processes more efficient; an error is thrown if the actual file size exceeds this " +
                    "value. Note: Actual limit is max size - 2"
      CAUSE         "The actual properties file size is larger than the specified expected maximum"
      RESOLUTION    "Reduce the size of the properties file or specify a larger maximum"
END_ERR



/* Error 53 */
class Repast_Error_53: public std::invalid_argument{
public:
  Repast_Error_53(std::string fileName): INVALID_ARG(ERROR_NUMBER 53)
      THROWN_BY     "Properties::readFile(const std::string& file, boost::mpi::communicator* comm, int maxPropFileSize)"
      REASON        "Properties file '" + fileName + "' not found."
      EXPLANATION   "The file specified as the properties file is not present"
      CAUSE         "The path or name of the file may be incorrect, or the file may be missing"
      RESOLUTION    "Specify the name of an existing file"
END_ERR


/* Error 54 */
class Repast_Error_54: public std::invalid_argument{
public:
  Repast_Error_54(const std::string line, const std::string file): INVALID_ARG(ERROR_NUMBER 54)
      THROWN_BY     "Properties::readFile(const std::string& file, boost::mpi::communicator* comm, int maxPropFileSize)"
      REASON        "Line '" + line + "' in Properties file '" + file + "' does not include '='"
      EXPLANATION   "Properties must be specified in 'KEY = value' (or 'KEY=value') format"
      CAUSE         "Properties file contains improperly formatted line."
      RESOLUTION    "Modify the incorrect line in the properties file."
END_ERR


/* Error 55 */
class Repast_Error_55: public std::invalid_argument{
public:
  Repast_Error_55(const std::string line, const std::string file): INVALID_ARG(ERROR_NUMBER 55)
      THROWN_BY     "Properties::readFile(const std::string& file, boost::mpi::communicator* comm, int maxPropFileSize)"
      REASON        "Line '" + line + "' in Properties file '" + file + "' does not include a key before the '='"
      EXPLANATION   "Properties must be specified in 'KEY = value' (or 'KEY=value') format" +
                    "both key and value are required."
      CAUSE         "Properties file contains improperly formatted line"
      RESOLUTION    "Modify the incorrect line in the properties file."
END_ERR


/* Error 56 */
class Repast_Error_56: public std::invalid_argument{
public:
  Repast_Error_56(const std::string line, const std::string file): INVALID_ARG(ERROR_NUMBER 56)
      THROWN_BY     "Properties::readFile(const std::string& file, boost::mpi::communicator* comm, int maxPropFileSize)"
      REASON        "Line '" + line + "' in Properties file '" + file + "' does not include a value after the '='"
      EXPLANATION   "Properties must be specified in 'KEY = value' (or 'KEY=value') format; " +
                    "both key and value are required."
      CAUSE         "Properties file contains improperly formatted line"
      RESOLUTION    "Modify the incorrect line in the properties file."
END_ERR

/* Error 57 */
class Repast_Error_57: public std::invalid_argument{
public:
  Repast_Error_57(): INVALID_ARG(ERROR_NUMBER 57)
      THROWN_BY     "initializeSeed(Properties& props, boost::mpi::communicator* comm)"
      REASON        "No communicator provided for use with global 'AUTO' seed."
      EXPLANATION   "'AUTO' specified for global.random.seed, but initializeSeed(Properties&, boost::mpi::communicator* = 0) is called with no communicator pointer. Automatically generated random seed (from process 0) cannot be shared to all processes."
      CAUSE         "Properties file cannot use AUTO for global.random.seed with this code"
      RESOLUTION    "Modify the incorrect line in the properties file, or alter the code to provide a communicator for initializeSeed"
END_ERR

/* TEMPLATE
class Repast_Error_: public std::invalid_argument{
public:
  Repast_Error_(): INVALID_ARG(ERROR_NUMBER 00)
      THROWN_BY     ""
      REASON        ""
      EXPLANATION   ""
      CAUSE         ""
      RESOLUTION    ""
END_ERR
*/

} // End namespace

#endif /* REPAST_ERRORS_H */
