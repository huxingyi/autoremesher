//
// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the OpenEXR Project.
//

#ifdef NDEBUG
#    undef NDEBUG
#endif

#include <testSize.h>
#include <testArithmetic.h>
#include <testError.h>
#include <testBitPatterns.h>
#include <testClassification.h>
#include <testLimits.h>
#include <testFunction.h>

#include <iostream>
#include <string.h>


#define TEST(x) if (argc < 2 || !strcmp (argv[1], #x)) x();


int
main (int argc, char *argv[])
{
    std::cout << "\ntesting type half:\n\n" << std::flush;

    TEST (testSize);
    TEST (testArithmetic);
    TEST (testNormalizedConversionError);
    TEST (testDenormalizedConversionError);
    TEST (testRoundingError);
    TEST (testBitPatterns);
    TEST (testClassification);
    TEST (testLimits);
    TEST (testFunction);

    return 0;
}
