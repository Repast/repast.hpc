/*
*Repast for High Performance Computing (Repast HPC)
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
*/

/*
 * properties_test.cpp
 *
 *  Created on: Sep 25, 2009
 *      Author: nick
 */

#include <gtest/gtest.h>

#include "repast_hpc/Properties.h"
#include <set>

using namespace repast;
using namespace std;

TEST(Properties, Load)
{
	repast::Properties props("./test.properties");
	ASSERT_EQ(3, props.size());

	set<string> expected;
	expected.insert("property.1");
	expected.insert("property.2");
	expected.insert("property.another.property");

	for (Properties::key_iterator iter  = props.keys_begin(); iter != props.keys_end(); iter++) {
		set<string>::iterator ex_iter = expected.find(*iter);
		ASSERT_TRUE(expected.end() != ex_iter);
		expected.erase(ex_iter);
	}

	ASSERT_EQ("23432", props.getProperty("property.1"));
	ASSERT_EQ("Hello, this is a property", props.getProperty("property.2"));
	ASSERT_EQ("3, 4, 5", props.getProperty("property.another.property"));
	ASSERT_EQ("", props.getProperty("fake.property"));

	props.putProperty("key", "abcd");
	ASSERT_EQ("abcd", props.getProperty("key"));
}
