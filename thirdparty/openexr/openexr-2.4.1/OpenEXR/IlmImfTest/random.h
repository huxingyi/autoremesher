// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the OpenEXR Project.

#pragma once

#include <limits>

extern int   random_int(int range = std::numeric_limits<int>::max());
extern float random_float(float range = std::numeric_limits<float>::max());
extern void  random_reseed (int s);





