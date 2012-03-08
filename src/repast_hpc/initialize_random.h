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
 *     Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *
 *     Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *
 *     Neither the name of the Argonne National Laboratory nor the names of its
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
 *  DistributionFactory.h
 *
 *  Created on: Oct 27, 2009
 *      Author: nick
 */

#ifndef INITRANDOM_H_
#define INITRANDOM_H_

#include <boost/mpi/communicator.hpp>

#include "Properties.h"

#define GLOBAL_RANDOM_SEED_PROPERTY "global.random.seed"
#define RANDOM_SEED_PROPERTY "random.seed"

namespace repast {

/**
 * Initializes the Random singleton with any properties (seed,
 * distributions) found in the properties object.
 */
void initializeRandom(Properties& props, boost::mpi::communicator* comm = 0);

/*
 * Generates a random number seed to use to initialize the random singleton
 *
 * Usage:
 *
 * Two properties can be specified in the properties object: "global.random.seed"
 * and "random.seed"
 *
 * If global.random.seed is used, it must include either a numeric value usable as a seed
 * OR the string 'AUTO'. If the numeric value is specified, this value is used as the
 * random number seed on all processes. If the 'AUTO' value is specified, a seed is generated
 * from system time on process 0 and communicated to all processes, so that all processes
 * use the same seed. Note that if 'AUTO is used, a communicator must be provided to this
 * function or an error will be thrown.
 *
 * If no global.random.seed is present in the properties collection, each processor will have
 * its own random number seed. The seed is determined by the value for random.seed
 * in the properties object and the presence or absence of a communicator object passed to
 * this function. The value of random.seed must be either a numeric value usable as a seed
 * or the string 'AUTO'; if no value for random.seed is specified, behavior is identical
 * to random.number=AUTO. If the 'AUTO' value is specified or the property is omitted, a seed is
 * generated from the system time; note that each process may have a different value for the
 * seed because the system time will not necessarily be synchronized (though it cannot also
 * be guaranteed that each process will have a unique value, and the values will likely be close
 * together and thus not be completely independent). If a communicator object is passed to
 * the function, the seeds used on each process will be different, but will be derived from
 * the seed on process 0.
 *
 * If either 'global.random.seed' or 'random.seed' is "AUTO" in the properties collection,
 * it is re-set with the value used for the seed; if both are absent, a random.seed value is added.
 * If the seeds are different on each process, the property value in each process's instance
 * will reflect the seed that it used; for seeds created using a communicator, the seed on
 * process '0' should be preferentially recorded because it can be used to recreate the
 * whole simulation run.
 *
 * In summary:
 *
 * If 'global.random.seed' IS in the properties collection:
 *
 * global.random.seed=<numeric>:
 *     All processes will use this value as the random number seed.
 *
 * global.random.seed=AUTO:
 *     A seed based on system time will be generated on proc 0 and sent to all other processes;
 *     a communicator must be passed to this function.
 *
 *
 * If 'global.random.seed' is NOT in the properties collection:
 *
 * random.seed=AUTO (or omitted)      & No communicator passed:
 *     All processes use local system time to create their random number seeds
 *
 * random.seed=<numeric>              & No communicator passed:
 *     All processes use the value of random.seed (if all properties collections are the
 *     same on all processes, the seed is effectively global).
 *
 * random.seed=AUTO (or omitted)      & Communicator passed:
 *     System time on process 0 is used to create a seed that is broadcast to all other processes;
 *     this value is then used by each process to derive different random seeds according to
 *     rank, and these are then used to initialize the random number generation system.
 *
 * random.seed=<numeric>              & Communicator passed:
 *     The random number seed in the properties collection will be used by each process to create
 *     different random seeds according to rank; these are then used to initialize the random
 *     number generation system.
 */
void initializeSeed(Properties& props, boost::mpi::communicator* comm);

}

#endif
