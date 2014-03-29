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
 * Zombie.cpp
 *
 *  Created on: Sep 1, 2010
 *      Author: nick
 */

#include "Zombie.h"
#include "Human.h"

#include "relogo/AgentSet.h"
#include "relogo/Patch.h"

using namespace repast::relogo;
using namespace repast;

struct CountHumansOnPatch {
	double operator()(const Patch* patch) const {
		AgentSet<Human> set;
		patch->turtlesOn(set);
		return set.size();
	}
};

void Zombie::step() {
    Patch* p = patchHere<Patch>();
    AgentSet<Patch> nghs = patchHere<Patch>()->neighbors<Patch>();
    Patch* winningPatch = nghs.maxOneOf(CountHumansOnPatch());
    face(winningPatch);
    move(.5);

    AgentSet<Human> humans;
    turtlesHere(humans);

    if (humans.size() > 0) {
      Human* human = humans.oneOf();
      infect(human);
    }
  }

  void Zombie::infect(Human* human) {
    human->infect();
    //_observer->createLink(this*, human, "Infection");
}
