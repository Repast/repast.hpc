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
 * grid_comp_test.cpp
 *
 *  Created on: Jun 23, 2009
 *      Author: nick
 */

#include "repast_hpc/matrix.h"
#include "repast_hpc/ValueLayer.h"
#include "repast_hpc/GridComponents.h"
#include "test.h"

#include <gtest/gtest.h>
#include <boost/unordered_set.hpp>
#include <stdlib.h>

using namespace repast;
using namespace std;

void testMatrix(Matrix<int>& matrix) {

	for (int x = 0; x < 10; x++) {
		for (int y = 0; y < 12; y++) {
			ASSERT_EQ(2, matrix[Point<int>(x, y)]);
			ASSERT_EQ(2, matrix.get(Point<int>(x, y)));
		}
	}

	int rnd[10][12];
	for (int x = 0; x < 10; x++) {
		for (int y = 0; y < 12; y++) {
			int val = rand();
			rnd[x][y] = val;
			matrix[Point<int>(x, y)] = val;
		}
	}

	for (int x = 0; x < 10; x++) {
		for (int y = 0; y < 12; y++) {
			ASSERT_EQ(rnd[x][y], matrix[Point<int>(x, y)]);
			ASSERT_EQ(rnd[x][y], matrix.get(Point<int>(x, y)));
		}
	}

	for (int x = 0; x < 10; x++) {
		for (int y = 0; y < 12; y++) {
			int val = rand();
			rnd[x][y] = val;
			matrix.set(val, Point<int>(x, y));
		}
	}

	for (int x = 0; x < 10; x++) {
		for (int y = 0; y < 12; y++) {
			ASSERT_EQ(rnd[x][y], matrix[Point<int>(x, y)]);
			ASSERT_EQ(rnd[x][y], matrix.get(Point<int>(x, y)));
		}
	}

	try {
		Point<int> pt(100, 1);
		matrix[pt];
		ASSERT_TRUE(false);
	} catch (out_of_range ex) {
		ASSERT_TRUE(true);
	}
}

void testCopy(Matrix<int>& one, Matrix<int>& two) {
	ASSERT_EQ(one.shape(), two.shape());
	for (int x = 0; x < 10; x++) {
		for (int y = 0; y < 12; y++) {
			ASSERT_EQ(one[Point<int>(x, y)], two[Point<int>(x, y)]);
		}
	}

}

TEST(Matrix, DenseMatrix)
{
	DenseMatrix<int> matrix(Point<int> (10, 12), 2);
	testMatrix(matrix);

	DenseMatrix<int> other = matrix;
	testCopy(matrix, other);

	DenseMatrix<int> other2(matrix);
	testCopy(matrix, other2);
}

TEST(Matrix, SparseMatrix)
{
	SparseMatrix<int> matrix(Point<int> (10, 12), 2);
	testMatrix(matrix);

	SparseMatrix<int> other = matrix;
	testCopy(matrix, other);

	SparseMatrix<int> other2(matrix);
	testCopy(matrix, other2);
}

void testCopy(DiscreteValueLayer<int, StrictBorders>& one, DiscreteValueLayer<int, StrictBorders>& two) {
	ASSERT_EQ(one.dimensions(), two.dimensions());
	ASSERT_EQ(one.name(), two.name());

	Point<double> origin = one.dimensions().origin();
	for (double x = origin.getX(); x < one.shape().getX() + origin.getX(); x++) {
		for (double y = origin.getY(); y < one.shape().getY() + origin.getY(); y++) {
			for (double z = origin.getZ(); z < one.shape().getZ() + origin.getZ(); z++) {
				ASSERT_EQ(one.get(Point<int>((int)x, (int)y, (int)z)), two.get(Point<int>((int)x, (int)y, (int)z)));
			}
		}
	}
}

void testCopy(ContinuousValueLayer<double, StrictBorders>& one, ContinuousValueLayer<double, StrictBorders>& two) {
	ASSERT_EQ(one.dimensions(), two.dimensions());
	ASSERT_EQ(one.name(), two.name());

	Point<double> origin = one.dimensions().origin();
	for (double x = origin.getX(); x < one.shape().getX() + origin.getX(); x++) {
		for (double y = origin.getY(); y < one.shape().getY() + origin.getY(); y++) {
			for (double z = origin.getZ(); z < one.shape().getZ() + origin.getZ(); z++) {
				ASSERT_EQ(one.get(Point<double>(x, y, z)), two.get(Point<double>(x, y, z)));
			}
		}
	}
}

void testCopy(DiscreteValueLayer<int, WrapAroundBorders>& one, DiscreteValueLayer<int, WrapAroundBorders>& two) {
	ASSERT_EQ(one.dimensions(), two.dimensions());
	ASSERT_EQ(one.name(), two.name());

	Point<double> origin = one.dimensions().origin();
	for (double x = origin.getX(); x < one.shape().getX() + origin.getX(); x++) {
		for (double y = origin.getY(); y < one.shape().getY() + origin.getY(); y++) {
			for (double z = origin.getZ(); z < one.shape().getZ() + origin.getZ(); z++) {
				ASSERT_EQ(one.get(Point<int>((int)x, (int)y, (int)z)), two.get(Point<int>((int)x, (int)y, (int)z)));
			}
		}
	}
}

void testCopy(ContinuousValueLayer<int, WrapAroundBorders>& one, ContinuousValueLayer<int, WrapAroundBorders>& two) {
	ASSERT_EQ(one.dimensions(), two.dimensions());
	ASSERT_EQ(one.name(), two.name());

	Point<double> origin = one.dimensions().origin();
	for (double x = origin.getX(); x < one.shape().getX() + origin.getX(); x++) {
		for (double y = origin.getY(); y < one.shape().getY() + origin.getY(); y++) {
			for (double z = origin.getZ(); z < one.shape().getZ() + origin.getZ(); z++) {
				ASSERT_EQ(one.get(Point<double>(x, y, z)), two.get(Point<double>(x, y, z)));
			}
		}
	}
}

TEST(ValueLayer, DiscreteZeroOrigin)
{
	DiscreteValueLayer<int, StrictBorders> vl("discrete", GridDimensions(Point<double> (4, 4, 10)), true, 6);
	for (int x = 0; x < 4; x++) {
		for (int y = 0; y < 4; y++) {
			for (int z = 0; z < 10; z++) {
				ASSERT_EQ(6, vl[Point<int>(x, y, z)]);
			}
		}
	}

	int rnd[4][4][10];
	for (int x = 0; x < 4; x++) {
		for (int y = 0; y < 4; y++) {
			for (int z = 0; z < 10; z++) {
				int val = rand();
				rnd[x][y][z] = val;
				vl.set(val, Point<int>(x, y, z));
			}
		}
	}

	for (int x = 0; x < 4; x++) {
		for (int y = 0; y < 4; y++) {
			for (int z = 0; z < 10; z++) {
				int val = rnd[x][y][z];
				ASSERT_EQ(val, vl[Point<int>(x, y, z)]);
				ASSERT_EQ(val, vl.get(Point<int>(x, y, z)));
			}
		}
	}

	for (int x = 0; x < 4; x++) {
		for (int y = 0; y < 4; y++) {
			for (int z = 0; z < 10; z++) {
				int val = rand();
				rnd[x][y][z] = val;
				vl[Point<int>(x, y, z)] = val;
			}
		}
	}

	for (int x = 0; x < 4; x++) {
		for (int y = 0; y < 4; y++) {
			for (int z = 0; z < 10; z++) {
				int val = rnd[x][y][z];
				ASSERT_EQ(val, vl[Point<int>(x, y, z)]);
				ASSERT_EQ(val, vl.get(Point<int>(x, y, z)));
			}
		}
	}

	DiscreteValueLayer<int, StrictBorders> other = vl;
	testCopy(vl, other);

	DiscreteValueLayer<int, StrictBorders> other2(vl);
	testCopy(vl, other2);
}

TEST(ValueLayer, ContinuousZeroOrigin)
{
	ContinuousValueLayer<double, StrictBorders> vl("discrete", GridDimensions(Point<double> (4, 4, 10)), 6.25);
	for (int x = 0; x < 4; x++) {
		for (int y = 0; y < 4; y++) {
			for (int z = 0; z < 10; z++) {
				ASSERT_EQ(6.25, vl[Point<double>(x, y, z)]);
			}
		}
	}

	vl[Point<double>(2.2, 3, 4.1)] = 12.3;
	ASSERT_EQ(12.3, vl[Point<double>(2.2, 3, 4.1)]);

	vl[Point<double>(0, 0, 0)] = 123.4;
	vl[Point<double>(3, 3, 9)] = 124334.234;
	ASSERT_EQ(123.4, vl[Point<double>(0, 0, 0)]);

	double rnd[4][4][10];
	for (int x = 0; x < 4; x++) {
		for (int y = 0; y < 4; y++) {
			for (int z = 0; z < 10; z++) {
				double val = rand();
				rnd[x][y][z] = val;
				vl[Point<double>(x, y, z)] = val;
			}
		}
	}

	for (int x = 0; x < 4; x++) {
		for (int y = 0; y < 4; y++) {
			for (int z = 0; z < 10; z++) {
				double val = rnd[x][y][z];
				ASSERT_EQ(val, vl[Point<double>(x, y, z)]);
				ASSERT_EQ(val, vl.get(Point<double>(x, y, z)));
			}
		}
	}

	for (int x = 0; x < 4; x++) {
		for (int y = 0; y < 4; y++) {
			for (int z = 0; z < 10; z++) {
				double val = rand();
				rnd[x][y][z] = val;
				vl.set(val, Point<double>(x, y, z));
			}
		}
	}

	for (int x = 0; x < 4; x++) {
		for (int y = 0; y < 4; y++) {
			for (int z = 0; z < 10; z++) {
				double val = rnd[x][y][z];
				ASSERT_EQ(val, vl[Point<double>(x, y, z)]);
				ASSERT_EQ(val, vl.get(Point<double>(x, y, z)));
			}
		}
	}

	ContinuousValueLayer<double, StrictBorders> other = vl;
	testCopy(vl, other);

	ContinuousValueLayer<double, StrictBorders> other2(vl);
	testCopy(vl, other2);

}

TEST(ValueLayer, DiscreteZeroOriginWrapped)
{
	DiscreteValueLayer<int, WrapAroundBorders> vl("discrete", GridDimensions(Point<double> (4, 4, 10)), true, 6);
	for (int x = 0; x < 4; x++) {
		for (int y = 0; y < 4; y++) {
			for (int z = 0; z < 10; z++) {
				ASSERT_EQ(6, vl[Point<int>(x, y, z)]);
			}
		}
	}

	int rnd[4][4][10];
	for (int x = 0; x < 4; x++) {
		for (int y = 0; y < 4; y++) {
			for (int z = 0; z < 10; z++) {
				int val = rand();
				rnd[x][y][z] = val;
				vl[Point<int>(x, y, z)] = val;
			}
		}
	}

	for (int x = 0; x < 4; x++) {
		for (int y = 0; y < 4; y++) {
			for (int z = 0; z < 10; z++) {
				int val = rnd[x][y][z];
				ASSERT_EQ(val, vl[Point<int>(x, y, z)]);
			}
		}
	}

	// wrap around
	ASSERT_EQ(rnd[2][2][2], vl[Point<int>(6, 6, 12)]);
	ASSERT_EQ(rnd[2][3][7], vl[Point<int>(-2, -1, -3)]);

	DiscreteValueLayer<int, WrapAroundBorders> other = vl;
	testCopy(vl, other);

	DiscreteValueLayer<int, WrapAroundBorders> other2(vl);
	testCopy(vl, other2);
}

TEST(ValueLayer, ContinuousZeroOriginWrapped)
{
	ContinuousValueLayer<int, WrapAroundBorders> vl("continuous", GridDimensions(Point<double> (4, 4, 10)), 6);
	for (int x = 0; x < 4; x++) {
		for (int y = 0; y < 4; y++) {
			for (int z = 0; z < 10; z++) {
				ASSERT_EQ(6, vl[Point<double>(x, y, z)]);
			}
		}
	}

	int rnd[4][4][10];
	for (int x = 0; x < 4; x++) {
		for (int y = 0; y < 4; y++) {
			for (int z = 0; z < 10; z++) {
				int val = rand();
				rnd[x][y][z] = val;
				vl[Point<double>(x, y, z)] = val;
			}
		}
	}

	for (int x = 0; x < 4; x++) {
		for (int y = 0; y < 4; y++) {
			for (int z = 0; z < 10; z++) {
				int val = rnd[x][y][z];
				ASSERT_EQ(val, vl[Point<double>(x, y, z)]);
			}
		}
	}

	// wrap around
	ASSERT_EQ(rnd[2][2][2], vl[Point<double>(6, 6, 12)]);
	ASSERT_EQ(rnd[2][3][7], vl[Point<double>(-2, -1, -3)]);

	ContinuousValueLayer<int, WrapAroundBorders> other = vl;
	testCopy(vl, other);

	ContinuousValueLayer<int, WrapAroundBorders> other2(vl);
	testCopy(vl, other2);
}

TEST(ValueLayer, DiscreteNonZeroOrigin)
{
	DiscreteValueLayer<int, StrictBorders> vl("discrete",
			GridDimensions(Point<double> (-2, -1, -2), Point<double> (4, 4, 10)), true, 6);
	Point<double> origin = vl.dimensions().origin();
	for (double x = origin.getX(); x < vl.shape().getX() + origin.getX(); x++) {
		for (double y = origin.getY(); y < vl.shape().getY() + origin.getY(); y++) {
			for (double z = origin.getZ(); z < vl.shape().getZ() + origin.getZ(); z++) {
				Point<int> pt((int)x, (int)y, (int)z);
				ASSERT_EQ(6, vl[pt]);
			}
		}
	}

	int rnd[4][4][10];
	for (double x = origin.getX(); x < vl.shape().getX() + origin.getX(); x++) {
		for (double y = origin.getY(); y < vl.shape().getY() + origin.getY(); y++) {
			for (double z = origin.getZ(); z < vl.shape().getZ() + origin.getZ(); z++) {

				int val = rand();
				rnd[(int)x - (int)origin.getX()][(int)y - (int)origin.getY()][(int)z - (int)origin.getZ()] = val;
				vl[Point<int>((int)x, (int)y, (int)z)] = val;
			}
		}
	}

	for (double x = origin.getX(); x < vl.shape().getX() + origin.getX(); x++) {
		for (double y = origin.getY(); y < vl.shape().getY() + origin.getY(); y++) {
			for (double z = origin.getZ(); z < vl.shape().getZ() + origin.getZ(); z++) {

				int val = rnd[(int)x - (int)origin.getX()][(int)y - (int)origin.getY()][(int)z - (int)origin.getZ()];
				ASSERT_EQ(val, vl[Point<int>(x, y, z)]);
			}
		}
	}

	DiscreteValueLayer<int, StrictBorders> other = vl;
	testCopy(vl, other);

	DiscreteValueLayer<int, StrictBorders> other2(vl);
	testCopy(vl, other2);

}

TEST(ValueLayer, ContinuousNonZeroOrigin)
{

	ContinuousValueLayer<double, StrictBorders> vl("continuous", GridDimensions(Point<double> (-2, -1, -2), Point<double> (4,
			4, 10)), 6);
	Point<double> origin = vl.dimensions().origin();
	for (double x = origin.getX(); x < vl.shape().getX() + origin.getX(); x++) {
		for (double y = origin.getY(); y < vl.shape().getY() + origin.getY(); y++) {
			for (double z = origin.getZ(); z < vl.shape().getZ() + origin.getZ(); z++) {
				Point<double> pt(x, y, z);
				ASSERT_EQ(6, vl[pt]);
			}
		}
	}

	double rnd[4][4][10];
	for (double x = origin.getX(); x < vl.shape().getX() + origin.getX(); x++) {
		for (double y = origin.getY(); y < vl.shape().getY() + origin.getY(); y++) {
			for (double z = origin.getZ(); z < vl.shape().getZ() + origin.getZ(); z++) {
				int val = rand();
				rnd[(int)x - (int)origin.getX()][(int)y - (int)origin.getY()][(int)z - (int)origin.getZ()] = val;
				vl[Point<double>(x, y, z)] = val;
			}
		}
	}

	for (double x = origin.getX(); x < vl.shape().getX() + origin.getX(); x++) {
		for (double y = origin.getY(); y < vl.shape().getY() + origin.getY(); y++) {
			for (double z = origin.getZ(); z < vl.shape().getZ() + origin.getZ(); z++) {
				double val = rnd[(int)x - (int)origin.getX()][(int)y - (int)origin.getY()][(int)z - (int)origin.getZ()];
				ASSERT_EQ(val, vl[Point<double>(x, y, z)]);
			}
		}
	}

	ContinuousValueLayer<double, StrictBorders> other = vl;
	testCopy(vl, other);

	ContinuousValueLayer<double, StrictBorders> other2(vl);
	testCopy(vl, other2);
}

