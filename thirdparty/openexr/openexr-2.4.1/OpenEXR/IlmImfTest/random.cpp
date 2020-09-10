// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the OpenEXR Project.

#include "random.h"
#include <random>
#include <assert.h>

static std::default_random_engine generator;

void
random_reseed (int s)
{
    generator.seed (s);
}

//
// Return an integer in the range [0,range), so that 0 <= i < range
//
int
random_int (int range)
{
    std::uniform_int_distribution<int> distribution (0, range-1);
    int i = distribution (generator);
    assert (0 <= i && i < range);
    return i;
}

//
// Return a float in the range [0, range), so that 0 <= f < range
//

float
random_float (float range)
{
    std::uniform_real_distribution<float> distribution (0, range);
    float f = distribution (generator);
    assert (0 <= f && f < range);
    return f;
}

    
