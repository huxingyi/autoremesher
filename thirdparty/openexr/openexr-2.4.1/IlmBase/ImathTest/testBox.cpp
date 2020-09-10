///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2010, Industrial Light & Magic, a division of Lucas
// Digital Ltd. LLC
// 
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
// *       Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
// *       Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
// *       Neither the name of Industrial Light & Magic nor the names of
// its contributors may be used to endorse or promote products derived
// from this software without specific prior written permission. 
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
///////////////////////////////////////////////////////////////////////////

#ifdef NDEBUG
#    undef NDEBUG
#endif

#include <testBoxAlgo.h>
#include "ImathBoxAlgo.h"
#include "ImathRandom.h"
#include <iostream>
#include <algorithm>
#include <assert.h>
#include <typeinfo>
#include <vector>

using namespace std;
using namespace IMATH_INTERNAL_NAMESPACE;

namespace {

//
// Test case generation utility - create a vector of IMATH_INTERNAL_NAMESPACE::Vec{2,3,4}
// with all permutations of integers 1..T::dimensions().
//
// Algorithm from www.bearcave.com/random_hacks/permute.html
//
template <class T>
static void
addItem(const std::vector<int> &value, std::vector<T> &perms)
{
    T p;
    for (unsigned int i = 0; i < value.size(); i++)
    {
        p[i] = value[i];
    }
    perms.push_back(p);
}

template <class T>
static void
visit(int &level, int n, int k, std::vector<int> &value, std::vector<T> &perms)
{
    level = level + 1;
    value[k] = level;

    if (level == n)
        addItem(value, perms);
    else
        for (int i = 0; i < n; i++)
            if (value[i] == 0)
                visit(level, n, i, value, perms);

    level = level - 1;
    value[k] = 0;
}


template <class T>
static void
permutations(std::vector<T> &perms)
{
    std::vector<int> value(T::dimensions());
    int level = -1;
    int n     = T::dimensions();

    visit(level, n, 0, value, perms);
}

template <class T>
static void
testConstructors(const char *type)
{
    cout << "    constructors for type " << type << endl;

    //
    // Empty
    //
    {
        IMATH_INTERNAL_NAMESPACE::Box<T> b;
        assert(b.min == T(T::baseTypeMax()) &&
               b.max == T(T::baseTypeMin()));
    }

    //
    // Single point
    //
    {
        T p;
        for (unsigned int i = 0; i < T::dimensions(); i++)
            p[i] = i;

        IMATH_INTERNAL_NAMESPACE::Box<T> b(p);
        assert(b.min == p && b.max == p);
    }

    //
    // Min and max
    //
    {
        T p0;
        T p1;
        for (unsigned int i = 0; i < T::dimensions(); i++)
        {
            p0[i] = i;
            p1[i] = 10 * T::dimensions() - i - 1;
        }

        IMATH_INTERNAL_NAMESPACE::Box<T> b(p0, p1);
        assert(b.min == p0 && b.max == p1);
    }
}

template <class T>
void
testMakeEmpty(const char *type)
{
    cout << "    makeEmpty() for type " << type << endl;

    //
    // Empty box
    //
    {
        IMATH_INTERNAL_NAMESPACE::Box<T> b;
        b.makeEmpty();
        assert(b.min == T(T::baseTypeMax()) &&
               b.max == T(T::baseTypeMin()));
    }

    //
    // Non-empty, has volume
    //
    {
        IMATH_INTERNAL_NAMESPACE::Box<T> b(T(-1), T(1));
        b.makeEmpty();
        assert(b.min == T(T::baseTypeMax()) &&
               b.max == T(T::baseTypeMin()));
    }

    //
    // Non-empty, no volume
    // Boxes are:
    //    2D: [(0, 0),       (0, 1)      ]
    //    3D: [(0, 0, 0),    (0, 0, 1)   ]
    //    4D: [(0, 0, 0, 0), (0, 0, 0, 1)]
    //
    {
        T min(0);
        T max(0);
        max[T::dimensions() - 1] = 1;

        IMATH_INTERNAL_NAMESPACE::Box<T> b(min, max);
        b.makeEmpty();
        assert(b.min == T(T::baseTypeMax()) &&
               b.max == T(T::baseTypeMin()));
    }
}

template <class T>
void
testMakeInfinite(const char *type)
{
    cout << "    makeInfinite() for type " << type << endl;

    //
    // Infinite box
    //
    {
        IMATH_INTERNAL_NAMESPACE::Box<T> b;
        b.makeInfinite();
        assert(b.min == T(T::baseTypeMin()) &&
               b.max == T(T::baseTypeMax()));
    }

    //
    // Non-empty, has volume
    //
    {
        IMATH_INTERNAL_NAMESPACE::Box<T> b(T(-1), T(1));
        b.makeInfinite();
        assert(b.min == T(T::baseTypeMin()) &&
               b.max == T(T::baseTypeMax()));
    }

    //
    // Non-empty, no volume
    // Boxes are:
    //    2D: [(0, 0),       (0, 1)      ]
    //    3D: [(0, 0, 0),    (0, 0, 1)   ]
    //    4D: [(0, 0, 0, 0), (0, 0, 0, 1)]
    //
    {
        T min(0);
        T max(0);
        max[T::dimensions() - 1] = 1;

        IMATH_INTERNAL_NAMESPACE::Box<T> b(min, max);
        b.makeInfinite();
        assert(b.min == T(T::baseTypeMin()) &&
               b.max == T(T::baseTypeMax()));
    }
}

template <class T>
void
testExtendByPoint(const char *type)
{
    cout << "    extendBy() point for type " << type << endl;

    IMATH_INTERNAL_NAMESPACE::Rand32 rand(0);

    const unsigned int iters = 10;

    //
    // Extend empty box with a single point.
    //
    for (unsigned int i = 0; i < iters; i++)
    {
        T p;
        for (unsigned int j = 0; j < T::dimensions(); j++)
            p[j] = typename T::BaseType (rand.nextf(-12345, 12345));
                              
        IMATH_INTERNAL_NAMESPACE::Box<T> b;
        b.extendBy(p);
        assert(b.min == p && b.max == p);
    }

    //
    // Extend empty box with a number of random points. Note that
    // this also covers extending a non-empty box.
    //
    for (unsigned int i = 0; i < iters; i++)
    {
        IMATH_INTERNAL_NAMESPACE::Box<T> b;

        T min;
        T max;

        for (unsigned int j = 0; j < i; j++)
        {
            T p;
            for (unsigned int k = 0; k < T::dimensions(); k++)
                p[k] =  typename T::BaseType (rand.nextf(-12345, 12345));

            if (j == 0)
            {
                min = p;
                max = p;
            }
            for (unsigned int k = 0; k < T::dimensions(); k++)
            {
                min[k] = std::min(min[k], p[k]);
                max[k] = std::max(max[k], p[k]);
            }

            b.extendBy(p);

            assert(b.min == min && b.max == max);
        }
    }
}

template <class T>
void
testExtendByBox(const char *type)
{
    cout << "    extendBy() box for type " << type << endl;

    //
    // Extend empty box with an empty box;
    //
    {
        IMATH_INTERNAL_NAMESPACE::Box<T> b;
        b.extendBy(IMATH_INTERNAL_NAMESPACE::Box<T>());
        assert(b.min == T(T::baseTypeMax()) &&
               b.max == T(T::baseTypeMin()));
    }

    //
    // Extend empty box with a non-empty box and vice versa.
    //
    {
        std::vector<T> perms;
        permutations(perms);

        for (unsigned int i = 0; i < perms.size(); i++)
        {
            for (unsigned int j = 0; j < perms.size(); j++)
            {
                T p0 = -perms[i];
                T p1 =  perms[j];

                IMATH_INTERNAL_NAMESPACE::Box<T> b0;
                b0.extendBy(IMATH_INTERNAL_NAMESPACE::Box<T>(p0, p1));
                assert(b0.min == p0 && b0.max == p1);

                IMATH_INTERNAL_NAMESPACE::Box<T> b1(p0, p1);
                b1.extendBy(IMATH_INTERNAL_NAMESPACE::Box<T>());
                assert(b1.min == p0 && b1.max == p1);
            }
        }
    }
        
    //
    // Extend non-empty box with non-empty box. Starts with empty, then builds.
    //
    IMATH_INTERNAL_NAMESPACE::Rand32 rand(0);
    const unsigned int iters = 10;
    {
        IMATH_INTERNAL_NAMESPACE::Box<T> b;

        T min, max;

        for (unsigned int i = 1; i < iters; i++)
        {
            T p0;
            T p1;
            for (unsigned int k = 0; k < T::dimensions(); k++)
            {
                p0[k] = typename T::BaseType (rand.nextf(   0,  999));
                p1[k] = typename T::BaseType (rand.nextf(1000, 1999));
            }

            min = b.min;
            max = b.max;
            for (unsigned int k = 0; k < T::dimensions(); k++)
            {
                min[k] = std::min(min[k], p0[k]);
                max[k] = std::max(max[k], p1[k]);
            }
            b.extendBy(IMATH_INTERNAL_NAMESPACE::Box<T>(p0, p1));

            assert(b.min == min && b.max == max);
        }
    }
}

template <class T>
void
testComparators(const char *type)
{
    cout << "    comparators for type " << type << endl;

    IMATH_INTERNAL_NAMESPACE::Rand32 rand(0);

    //
    // Compare empty.
    //
    {
        IMATH_INTERNAL_NAMESPACE::Box<T> b0;
        IMATH_INTERNAL_NAMESPACE::Box<T> b1;

        assert(b0 == b1);
        assert(!(b0 != b1));
    }

    //
    // Compare empty to non-empty.
    //
    {
        std::vector<T> perms;
        permutations(perms);

        for (unsigned int i = 0; i < perms.size(); i++)
        {
            for (unsigned int j = 0; j < perms.size(); j++)
            {
                T p0 = -perms[i];
                T p1 =  perms[j];

                IMATH_INTERNAL_NAMESPACE::Box<T> b0;
                IMATH_INTERNAL_NAMESPACE::Box<T> b1(p0, p1);
                assert(!(b0 == b1));
                assert(b0 != b1);
            }
        }
    }

    //
    // Compare two non-empty
    //
    {
        std::vector<T> perms;
        permutations(perms);

        for (unsigned int i = 0; i < perms.size(); i++)
        {
            for (unsigned int j = 0; j < perms.size(); j++)
            {
                T p0 = -perms[i];
                T p1 =  perms[j];

                T p2 = -perms[j];
                T p3 =  perms[i];

                IMATH_INTERNAL_NAMESPACE::Box<T> b0(p0, p1);
                IMATH_INTERNAL_NAMESPACE::Box<T> b1(p2, p3);
                IMATH_INTERNAL_NAMESPACE::Box<T> b2(p0, p1);

                if (i == j)
                {
                    assert(b0 == b1);
                    assert(!(b0 != b1));
                }
                else
                {
                    assert(b0 != b1);
                    assert(!(b0 == b1));
                }
                assert(b0 == b2);
                assert(!(b0 != b2));
            }
        }
    }
}


template <class T>
void
testIntersects(const char *type)
{
    cout << "    intersects() for type " << type << endl;

    IMATH_INTERNAL_NAMESPACE::Rand32 rand(0);

    //
    // Intersect point with empty box.
    //
    {
        IMATH_INTERNAL_NAMESPACE::Box<T> b;
        T             p(1);

        assert(!b.intersects(p));
    }

    //
    // Intersect point with non-empty, has-volume box.
    //
    {
        IMATH_INTERNAL_NAMESPACE::Box<T> b(T(-1), T(1));
        T             p0(0);
        T             p1(5);
        T             p2(-5);

        assert(b.intersects(p0));
        assert(!b.intersects(p1));
        assert(!b.intersects(p2));
    }

    //
    // Intersect point with non-empty, no-volume box.
    // Boxes are:
    //    2D: [(0, 0),       (0, 1)      ]
    //    3D: [(0, 0, 0),    (0, 0, 1)   ]
    //    4D: [(0, 0, 0, 0), (0, 0, 0, 1)]
    //
    {
        T min(0);
        T max = min;
        max[T::dimensions() - 1] = 1;

        T p0(0);
        T p1(5);
        IMATH_INTERNAL_NAMESPACE::Box<T> b(min, max);

        assert(b.intersects(p0));
        assert(!b.intersects(p1));
    }

    //
    // Intersect empty box with empty box.
    //
    {
        IMATH_INTERNAL_NAMESPACE::Box<T> b0;
        IMATH_INTERNAL_NAMESPACE::Box<T> b1;

        assert(!b0.intersects(b1));
        assert(!b1.intersects(b0));
    }

    //
    // Intersect empty box with non-empty has-volume boxes.
    //
    {
        IMATH_INTERNAL_NAMESPACE::Box<T> b0;
        IMATH_INTERNAL_NAMESPACE::Box<T> b1(T(-1), T(1));
        IMATH_INTERNAL_NAMESPACE::Box<T> b2(T( 1), T(2));

        assert(!b0.intersects(b1));
        assert(!b0.intersects(b2));

        assert(!b1.intersects(b0));
        assert(!b2.intersects(b0));
    }

    //
    // Intersect empty box with non-empty no-volume box.
    // Boxes are:
    //    2D: [(0, 0),       (0, 1)      ]
    //    3D: [(0, 0, 0),    (0, 0, 1)   ]
    //    4D: [(0, 0, 0, 0), (0, 0, 0, 1)]
    //
    {
        T min(0);
        T max = min;
        max[T::dimensions() - 1] = 1;

        IMATH_INTERNAL_NAMESPACE::Box<T> b0;
        IMATH_INTERNAL_NAMESPACE::Box<T> b1(min, max);

        assert(!b0.intersects(b1));
        assert(!b1.intersects(b0));
    }

    //
    // Intersect non-empty has-volume box with non-empty has-volume box.
    //
    {
        IMATH_INTERNAL_NAMESPACE::Box<T> b1(T(-1), T(1));
        IMATH_INTERNAL_NAMESPACE::Box<T> b2(T(-1), T(1));
        IMATH_INTERNAL_NAMESPACE::Box<T> b3(T( 1), T(2));
        IMATH_INTERNAL_NAMESPACE::Box<T> b4(T( 2), T(3));

        assert(b1.intersects(b1));
        assert(b1.intersects(b3));
        assert(!b1.intersects(b4));

        assert(b3.intersects(b1));
        assert(!b4.intersects(b1));
    }

    //
    // Intersect non-empty has-volume box with non-empty no-volume box.
    //
    // Boxes are:
    //    2D: [(0, 0),       (0, 1)      ]
    //    3D: [(0, 0, 0),    (0, 0, 1)   ]
    //    4D: [(0, 0, 0, 0), (0, 0, 0, 1)]
    //
    {
        IMATH_INTERNAL_NAMESPACE::Box<T> b0(T(-1), T(1));

        T min(0);
        T max = min;
        max[T::dimensions() - 1] = 1;

        IMATH_INTERNAL_NAMESPACE::Box<T> b1(min, max);
        IMATH_INTERNAL_NAMESPACE::Box<T> b2(min + T(2), max + T(2));
        
        assert(b0.intersects(b1));
        assert(b1.intersects(b0));

        assert(!b0.intersects(b2));
        assert(!b2.intersects(b1));
    }

    //
    // Intersect non-empty no-volume box with non-empty no-volume box.
    //
    // Boxes are:
    //    2D: [(0, 0),       (0, 1)      ]
    //    3D: [(0, 0, 0),    (0, 0, 1)   ]
    //    4D: [(0, 0, 0, 0), (0, 0, 0, 1)]
    //
    {
        T min(0);
        T max = min;
        max[T::dimensions() - 1] = 1;

        IMATH_INTERNAL_NAMESPACE::Box<T> b0(min, max);
        IMATH_INTERNAL_NAMESPACE::Box<T> b1(min,  max + T(2));
        IMATH_INTERNAL_NAMESPACE::Box<T> b2(min + T(2),  max + T(2));
        
        assert(b0.intersects(b1));
        assert(b1.intersects(b0));

        assert(!b0.intersects(b2));
        assert(!b2.intersects(b0));
    }
}

template <class T>
void
testSize(const char *type)
{
    cout << "    size() for type " << type << endl;

    //
    // Size of empty box.
    //
    {
        IMATH_INTERNAL_NAMESPACE::Box<T> b;
        assert(b.size() == T(0));
    }

    //
    // Size of non-empty, has-volume box.
    // Boxes are:
    //    2D: [(-1, -1),         (1, 1)       ]
    //    3D: [(-1, -1, -1),     (1, 1, 1)    ]
    //    4D: [(-1, -1, -1, -1), (1, 1, 1, 1) ] 
    //
    // and
    //
    //    2D: [(-1, -2),         (1, 2)       ]
    //    3D: [(-1, -2, -3),     (1, 2, 3)    ]
    //    4D: [(-1, -2, -3, -4), (1, 2, 3, 4) ]
    //
    {
        IMATH_INTERNAL_NAMESPACE::Box<T> b0(T(-1), T(1));
        assert(b0.size() == T(2));

        T p;
        for (unsigned int i = 0; i < T::dimensions(); i++)
        {
            p[i] = i;
        }
        IMATH_INTERNAL_NAMESPACE::Box<T> b1(-p, p);
        assert(b1.size() == p * T(2));
    }

    //
    // Size of non-empty, no-volume box.
    // Boxes are:
    //    2D: [(0, 0),       (0, 1)      ]
    //    3D: [(0, 0, 0),    (0, 0, 1)   ]
    //    4D: [(0, 0, 0, 0), (0, 0, 0, 1)]
    //
    {
        T min(0);
        T max = min;
        max[T::dimensions() - 1] = 1;

        IMATH_INTERNAL_NAMESPACE::Box<T> b(min, max);

        assert(b.size() == max);
    }
}


template <class T>
void
testCenter(const char *type)
{
    cout << "    center() for type " << type << endl;

    //
    // Center of empty box.
    //
    {
        IMATH_INTERNAL_NAMESPACE::Box<T> b;
        assert(b.center() == T(0));
    }

    //
    // Center of non-empty, has-volume box.
    // Boxes are:
    //    2D: [(-1, -1),         (1, 1)       ]
    //    3D: [(-1, -1, -1),     (1, 1, 1)    ]
    //    4D: [(-1, -1, -1, -1), (1, 1, 1, 1) ] 
    //
    // and
    //
    //    2D: [(-2, -4),         ( 8,  2)       ]
    //    3D: [(-2, -4, -6),     (12,  8, 2)    ]
    //    4D: [(-2, -4, -6, -8), (16, 12, 8, 4) ]
    //
    {
        IMATH_INTERNAL_NAMESPACE::Box<T> b0(T(-1), T(1));
        assert(b0.center() == T(0));

        T p0;
        T p1;
        for (unsigned int i = 0; i < T::dimensions(); i++)
        {
            p0[i] = -typename T::BaseType(1 << (i + 1));
            p1[i] =  typename T::BaseType(1 << (T::dimensions() - i));
        }
        IMATH_INTERNAL_NAMESPACE::Box<T> b1(p0, p1);
        assert(b1.center() == (p1 + p0) / 2);
    }

    //
    // Center of non-empty, no-volume box.
    // Boxes are:
    //    2D: [(0, 0),       (0, 2)      ]
    //    3D: [(0, 0, 0),    (0, 0, 2)   ]
    //    4D: [(0, 0, 0, 0), (0, 0, 0, 2)]
    //
    {
        T min(0);
        T max = min;
        max[T::dimensions() - 1] = 2;

        IMATH_INTERNAL_NAMESPACE::Box<T> b(min, max);

        assert(b.center() == max /2);
    }
}


template <class T>
void
testIsEmpty(const char *type)
{
    cout << "    isEmpty() for type " << type << endl;

    //
    // Empty box.
    //
    {
        IMATH_INTERNAL_NAMESPACE::Box<T> b;
        assert(b.isEmpty());
    }

    //
    // Non-empty, has-volume box.
    //    2D: [(-2, -4),         ( 8,  2)       ]
    //    3D: [(-2, -4, -6),     (12,  8, 2)    ]
    //    4D: [(-2, -4, -6, -8), (16, 12, 8, 4) ]
    //
    {
        IMATH_INTERNAL_NAMESPACE::Box<T> b0(T(-1), T(1));
        assert(!b0.isEmpty());

        T p0;
        T p1;
        for (unsigned int i = 0; i < T::dimensions(); i++)
        {
           p0[i] = -typename T::BaseType(1 << (i + 1));
           p1[i] =  typename T::BaseType(1 << (T::dimensions() - i));
        }
        IMATH_INTERNAL_NAMESPACE::Box<T> b1(p0, p1);
        assert(!b1.isEmpty());
    }

    //
    // Non-empty, no-volume box.
    // Boxes are:
    //    2D: [(0, 0),       (0, 2)      ]
    //    3D: [(0, 0, 0),    (0, 0, 2)   ]
    //    4D: [(0, 0, 0, 0), (0, 0, 0, 2)]
    //
    {
        T min(0);
        T max = min;
        max[T::dimensions() - 1] = 2;

        IMATH_INTERNAL_NAMESPACE::Box<T> b(min, max);

        assert(!b.isEmpty());
    }
}


template <class T>
void
testIsInfinite(const char *type)
{
    cout << "    isInfinite() for type " << type << endl;

    //
    // Infinite box.
    //
    {
        IMATH_INTERNAL_NAMESPACE::Box<T> b;
        b.makeInfinite();
        assert(b.isInfinite());
    }

    //
    // Non-empty, has-volume box.
    //    2D: [(-2, -4),         ( 8,  2)       ]
    //    3D: [(-2, -4, -6),     (12,  8, 2)    ]
    //    4D: [(-2, -4, -6, -8), (16, 12, 8, 4) ]
    //
    {
        IMATH_INTERNAL_NAMESPACE::Box<T> b0(T(-1), T(1));
        assert(!b0.isInfinite());

        T p0;
        T p1;
        for (unsigned int i = 0; i < T::dimensions(); i++)
        {
            p0[i] = -typename T::BaseType(1 << (i + 1));
            p1[i] =  typename T::BaseType(1 << (T::dimensions() - i));
        }
        IMATH_INTERNAL_NAMESPACE::Box<T> b1(p0, p1);
        assert(!b1.isInfinite());
    }

    //
    // Non-empty, no-volume box.
    // Boxes are:
    //    2D: [(0, 0),       (0, 2)      ]
    //    3D: [(0, 0, 0),    (0, 0, 2)   ]
    //    4D: [(0, 0, 0, 0), (0, 0, 0, 2)]
    //
    {
        T min(0);
        T max = min;
        max[T::dimensions() - 1] = 2;

        IMATH_INTERNAL_NAMESPACE::Box<T> b(min, max);

        assert(!b.isInfinite());
    }
}


template <class T>
void
testHasVolume(const char *type)
{
    cout << "    hasVolume() for type " << type << endl;

    //
    // Empty box.
    //
    {
        IMATH_INTERNAL_NAMESPACE::Box<T> b;
        assert(!b.hasVolume());
    }

    //
    // Infinite box.
    //
    {
        IMATH_INTERNAL_NAMESPACE::Box<T> b;
        b.makeInfinite();
        assert(b.hasVolume());
    }

    //
    // Non-empty, has-volume box.
    //    2D: [(-2, -4),         ( 8,  2)       ]
    //    3D: [(-2, -4, -6),     (12,  8, 2)    ]
    //    4D: [(-2, -4, -6, -8), (16, 12, 8, 4) ]
    //
    {
        IMATH_INTERNAL_NAMESPACE::Box<T> b0(T(-1), T(1));
        assert(b0.hasVolume());

        T p0;
        T p1;
        for (unsigned int i = 0; i < T::dimensions(); i++)
        {
            p0[i] = -typename T::BaseType(1 << (i + 1));
            p1[i] =  typename T::BaseType(1 << (T::dimensions() - i));
        }
        IMATH_INTERNAL_NAMESPACE::Box<T> b1(p0, p1);
        assert(b1.hasVolume());
    }

    //
    // Non-empty, no-volume box.
    // Boxes are:
    //    2D: [(0, 0),       (0, 2)      ]
    //    3D: [(0, 0, 0),    (0, 0, 2)   ]
    //    4D: [(0, 0, 0, 0), (0, 0, 0, 2)]
    //
    {
        T min(0);
        T max = min;
        max[T::dimensions() - 1] = 2;

        IMATH_INTERNAL_NAMESPACE::Box<T> b(min, max);

        assert(!b.hasVolume());
    }
}


template <class T>
void
testMajorAxis(const char *type)
{
    cout << "    majorAxis() for type " << type << endl;

    //
    // Empty box.
    //
    {
        IMATH_INTERNAL_NAMESPACE::Box<T> b;
        assert(b.majorAxis() == 0);
    }

    //
    // Non-empty, has-volume box.
    // Boxes are [ (0, 0, ...), (<all permutations of 1..T::dimensions()>) ]
    //
    {
        std::vector<T> perms;
        permutations(perms);

        for (unsigned int i = 0; i < perms.size(); i++)
        {
            IMATH_INTERNAL_NAMESPACE::Box<T> b(T(0), perms[i]);

            unsigned int major = 0;
            T size = perms[i] - T(0);
            for (unsigned int j = 1; j < T::dimensions(); j++)
                if (size[j] > size[major])
                    major = j;
                    
            assert(b.majorAxis() == major);
        }
    }

    //
    // Non-empty, no-volume box.
    // Boxes are:
    //    2D: [(0, 0), (1, 0) ]
    //    2D: [(0, 0), (0, 1) ]
    //
    //    3D: [(0, 0), (1, 0, 0) ]
    //    3D: [(0, 0), (0, 1, 0) ]
    //    3D: [(0, 0), (0, 0, 1) ]
    //
    //    and similarly for 4D
    //
    {
        for (unsigned int i = 0; i < T::dimensions(); i++)
        {
            for (unsigned int j = 0; j < T::dimensions(); j++)
            {
                T max(0);
                max[j] = 1;

                IMATH_INTERNAL_NAMESPACE::Box<T> b(T(0), max);
                assert(b.majorAxis() == j);
            }
        }
    }
}

} // anonymous namespace


void
testBox()
{
    cout << "Testing box methods" << endl;

    //
    // Constructors
    //
    testConstructors<IMATH_INTERNAL_NAMESPACE::V2s>("V2s");
    testConstructors<IMATH_INTERNAL_NAMESPACE::V2i>("V2i");
    testConstructors<IMATH_INTERNAL_NAMESPACE::V2f>("V2f");
    testConstructors<IMATH_INTERNAL_NAMESPACE::V2d>("V2d");

    testConstructors<IMATH_INTERNAL_NAMESPACE::V3s>("V3s");
    testConstructors<IMATH_INTERNAL_NAMESPACE::V3i>("V3i");
    testConstructors<IMATH_INTERNAL_NAMESPACE::V3f>("V3f");
    testConstructors<IMATH_INTERNAL_NAMESPACE::V3d>("V3d");

    testConstructors<IMATH_INTERNAL_NAMESPACE::V4s>("V4s");
    testConstructors<IMATH_INTERNAL_NAMESPACE::V4i>("V4i");
    testConstructors<IMATH_INTERNAL_NAMESPACE::V4f>("V4f");
    testConstructors<IMATH_INTERNAL_NAMESPACE::V4d>("V4d");

    //
    // makeEmpty()
    //
    testMakeEmpty<IMATH_INTERNAL_NAMESPACE::V2s>("V2s");
    testMakeEmpty<IMATH_INTERNAL_NAMESPACE::V2i>("V2i");
    testMakeEmpty<IMATH_INTERNAL_NAMESPACE::V2f>("V2f");
    testMakeEmpty<IMATH_INTERNAL_NAMESPACE::V2d>("V2d");

    testMakeEmpty<IMATH_INTERNAL_NAMESPACE::V3s>("V3s");
    testMakeEmpty<IMATH_INTERNAL_NAMESPACE::V3i>("V3i");
    testMakeEmpty<IMATH_INTERNAL_NAMESPACE::V3f>("V3f");
    testMakeEmpty<IMATH_INTERNAL_NAMESPACE::V3d>("V3d");

    testMakeEmpty<IMATH_INTERNAL_NAMESPACE::V4s>("V4s");
    testMakeEmpty<IMATH_INTERNAL_NAMESPACE::V4i>("V4i");
    testMakeEmpty<IMATH_INTERNAL_NAMESPACE::V4f>("V4f");
    testMakeEmpty<IMATH_INTERNAL_NAMESPACE::V4d>("V4d");

    //
    // makeInfinite()
    //
    testMakeInfinite<IMATH_INTERNAL_NAMESPACE::V2s>("V2s");
    testMakeInfinite<IMATH_INTERNAL_NAMESPACE::V2i>("V2i");
    testMakeInfinite<IMATH_INTERNAL_NAMESPACE::V2f>("V2f");
    testMakeInfinite<IMATH_INTERNAL_NAMESPACE::V2d>("V2d");

    testMakeInfinite<IMATH_INTERNAL_NAMESPACE::V3s>("V3s");
    testMakeInfinite<IMATH_INTERNAL_NAMESPACE::V3i>("V3i");
    testMakeInfinite<IMATH_INTERNAL_NAMESPACE::V3f>("V3f");
    testMakeInfinite<IMATH_INTERNAL_NAMESPACE::V3d>("V3d");

    testMakeInfinite<IMATH_INTERNAL_NAMESPACE::V4s>("V4s");
    testMakeInfinite<IMATH_INTERNAL_NAMESPACE::V4i>("V4i");
    testMakeInfinite<IMATH_INTERNAL_NAMESPACE::V4f>("V4f");
    testMakeInfinite<IMATH_INTERNAL_NAMESPACE::V4d>("V4d");

    //
    // extendBy() (point)
    //
    testExtendByPoint<IMATH_INTERNAL_NAMESPACE::V2s>("V2s");
    testExtendByPoint<IMATH_INTERNAL_NAMESPACE::V2i>("V2i");
    testExtendByPoint<IMATH_INTERNAL_NAMESPACE::V2f>("V2f");
    testExtendByPoint<IMATH_INTERNAL_NAMESPACE::V2d>("V2d");

    testExtendByPoint<IMATH_INTERNAL_NAMESPACE::V3s>("V3s");
    testExtendByPoint<IMATH_INTERNAL_NAMESPACE::V3i>("V3i");
    testExtendByPoint<IMATH_INTERNAL_NAMESPACE::V3f>("V3f");
    testExtendByPoint<IMATH_INTERNAL_NAMESPACE::V3d>("V3d");

    testExtendByPoint<IMATH_INTERNAL_NAMESPACE::V4s>("V4s");
    testExtendByPoint<IMATH_INTERNAL_NAMESPACE::V4i>("V4i");
    testExtendByPoint<IMATH_INTERNAL_NAMESPACE::V4f>("V4f");
    testExtendByPoint<IMATH_INTERNAL_NAMESPACE::V4d>("V4d");

    //
    // extendBy() box
    //
    testExtendByBox<IMATH_INTERNAL_NAMESPACE::V2s>("V2s");
    testExtendByBox<IMATH_INTERNAL_NAMESPACE::V2i>("V2i");
    testExtendByBox<IMATH_INTERNAL_NAMESPACE::V2f>("V2f");
    testExtendByBox<IMATH_INTERNAL_NAMESPACE::V2d>("V2d");

    testExtendByBox<IMATH_INTERNAL_NAMESPACE::V3s>("V3s");
    testExtendByBox<IMATH_INTERNAL_NAMESPACE::V3i>("V3i");
    testExtendByBox<IMATH_INTERNAL_NAMESPACE::V3f>("V3f");
    testExtendByBox<IMATH_INTERNAL_NAMESPACE::V3d>("V3d");

    testExtendByBox<IMATH_INTERNAL_NAMESPACE::V4s>("V4s");
    testExtendByBox<IMATH_INTERNAL_NAMESPACE::V4i>("V4i");
    testExtendByBox<IMATH_INTERNAL_NAMESPACE::V4f>("V4f");
    testExtendByBox<IMATH_INTERNAL_NAMESPACE::V4d>("V4d");

    //
    // == and !==
    //
    testComparators<IMATH_INTERNAL_NAMESPACE::V2s>("V2s");
    testComparators<IMATH_INTERNAL_NAMESPACE::V2i>("V2i");
    testComparators<IMATH_INTERNAL_NAMESPACE::V2f>("V2f");
    testComparators<IMATH_INTERNAL_NAMESPACE::V2d>("V2d");

    testComparators<IMATH_INTERNAL_NAMESPACE::V3s>("V3s");
    testComparators<IMATH_INTERNAL_NAMESPACE::V3i>("V3i");
    testComparators<IMATH_INTERNAL_NAMESPACE::V3f>("V3f");
    testComparators<IMATH_INTERNAL_NAMESPACE::V3d>("V3d");

    testComparators<IMATH_INTERNAL_NAMESPACE::V4s>("V4s");
    testComparators<IMATH_INTERNAL_NAMESPACE::V4i>("V4i");
    testComparators<IMATH_INTERNAL_NAMESPACE::V4f>("V4f");
    testComparators<IMATH_INTERNAL_NAMESPACE::V4d>("V4d");

    //
    // size()
    //
    testSize<IMATH_INTERNAL_NAMESPACE::V2s>("V2s");
    testSize<IMATH_INTERNAL_NAMESPACE::V2i>("V2i");
    testSize<IMATH_INTERNAL_NAMESPACE::V2f>("V2f");
    testSize<IMATH_INTERNAL_NAMESPACE::V2d>("V2d");

    testSize<IMATH_INTERNAL_NAMESPACE::V3s>("V3s");
    testSize<IMATH_INTERNAL_NAMESPACE::V3i>("V3i");
    testSize<IMATH_INTERNAL_NAMESPACE::V3f>("V3f");
    testSize<IMATH_INTERNAL_NAMESPACE::V3d>("V3d");

    testSize<IMATH_INTERNAL_NAMESPACE::V4s>("V4s");
    testSize<IMATH_INTERNAL_NAMESPACE::V4i>("V4i");
    testSize<IMATH_INTERNAL_NAMESPACE::V4f>("V4f");
    testSize<IMATH_INTERNAL_NAMESPACE::V4d>("V4d");

    //
    // center()
    //
    testCenter<IMATH_INTERNAL_NAMESPACE::V2s>("V2s");
    testCenter<IMATH_INTERNAL_NAMESPACE::V2i>("V2i");
    testCenter<IMATH_INTERNAL_NAMESPACE::V2f>("V2f");
    testCenter<IMATH_INTERNAL_NAMESPACE::V2d>("V2d");

    testCenter<IMATH_INTERNAL_NAMESPACE::V3s>("V3s");
    testCenter<IMATH_INTERNAL_NAMESPACE::V3i>("V3i");
    testCenter<IMATH_INTERNAL_NAMESPACE::V3f>("V3f");
    testCenter<IMATH_INTERNAL_NAMESPACE::V3d>("V3d");

    testCenter<IMATH_INTERNAL_NAMESPACE::V4s>("V4s");
    testCenter<IMATH_INTERNAL_NAMESPACE::V4i>("V4i");
    testCenter<IMATH_INTERNAL_NAMESPACE::V4f>("V4f");
    testCenter<IMATH_INTERNAL_NAMESPACE::V4d>("V4d");

    //
    // isEmpty()
    //
    testIsEmpty<IMATH_INTERNAL_NAMESPACE::V2s>("V2s");
    testIsEmpty<IMATH_INTERNAL_NAMESPACE::V2i>("V2i");
    testIsEmpty<IMATH_INTERNAL_NAMESPACE::V2f>("V2f");
    testIsEmpty<IMATH_INTERNAL_NAMESPACE::V2d>("V2d");

    testIsEmpty<IMATH_INTERNAL_NAMESPACE::V3s>("V3s");
    testIsEmpty<IMATH_INTERNAL_NAMESPACE::V3i>("V3i");
    testIsEmpty<IMATH_INTERNAL_NAMESPACE::V3f>("V3f");
    testIsEmpty<IMATH_INTERNAL_NAMESPACE::V3d>("V3d");

    testIsEmpty<IMATH_INTERNAL_NAMESPACE::V4s>("V4s");
    testIsEmpty<IMATH_INTERNAL_NAMESPACE::V4i>("V4i");
    testIsEmpty<IMATH_INTERNAL_NAMESPACE::V4f>("V4f");
    testIsEmpty<IMATH_INTERNAL_NAMESPACE::V4d>("V4d");

    //
    // isInfinite()
    //
    testIsInfinite<IMATH_INTERNAL_NAMESPACE::V2s>("V2s");
    testIsInfinite<IMATH_INTERNAL_NAMESPACE::V2i>("V2i");
    testIsInfinite<IMATH_INTERNAL_NAMESPACE::V2f>("V2f");
    testIsInfinite<IMATH_INTERNAL_NAMESPACE::V2d>("V2d");

    testIsInfinite<IMATH_INTERNAL_NAMESPACE::V3s>("V3s");
    testIsInfinite<IMATH_INTERNAL_NAMESPACE::V3i>("V3i");
    testIsInfinite<IMATH_INTERNAL_NAMESPACE::V3f>("V3f");
    testIsInfinite<IMATH_INTERNAL_NAMESPACE::V3d>("V3d");

    testIsInfinite<IMATH_INTERNAL_NAMESPACE::V4s>("V4s");
    testIsInfinite<IMATH_INTERNAL_NAMESPACE::V4i>("V4i");
    testIsInfinite<IMATH_INTERNAL_NAMESPACE::V4f>("V4f");
    testIsInfinite<IMATH_INTERNAL_NAMESPACE::V4d>("V4d");

    //
    // hasVolume()
    //
    testHasVolume<IMATH_INTERNAL_NAMESPACE::V2s>("V2s");
    testHasVolume<IMATH_INTERNAL_NAMESPACE::V2i>("V2i");
    testHasVolume<IMATH_INTERNAL_NAMESPACE::V2f>("V2f");
    testHasVolume<IMATH_INTERNAL_NAMESPACE::V2d>("V2d");

    testHasVolume<IMATH_INTERNAL_NAMESPACE::V3s>("V3s");
    testHasVolume<IMATH_INTERNAL_NAMESPACE::V3i>("V3i");
    testHasVolume<IMATH_INTERNAL_NAMESPACE::V3f>("V3f");
    testHasVolume<IMATH_INTERNAL_NAMESPACE::V3d>("V3d");

    testHasVolume<IMATH_INTERNAL_NAMESPACE::V4s>("V4s");
    testHasVolume<IMATH_INTERNAL_NAMESPACE::V4i>("V4i");
    testHasVolume<IMATH_INTERNAL_NAMESPACE::V4f>("V4f");
    testHasVolume<IMATH_INTERNAL_NAMESPACE::V4d>("V4d");

    //
    // majorAxis()
    //
    testMajorAxis<IMATH_INTERNAL_NAMESPACE::V2s>("V2s");
    testMajorAxis<IMATH_INTERNAL_NAMESPACE::V2i>("V2i");
    testMajorAxis<IMATH_INTERNAL_NAMESPACE::V2f>("V2f");
    testMajorAxis<IMATH_INTERNAL_NAMESPACE::V2d>("V2d");

    testMajorAxis<IMATH_INTERNAL_NAMESPACE::V3s>("V3s");
    testMajorAxis<IMATH_INTERNAL_NAMESPACE::V3i>("V3i");
    testMajorAxis<IMATH_INTERNAL_NAMESPACE::V3f>("V3f");
    testMajorAxis<IMATH_INTERNAL_NAMESPACE::V3d>("V3d");

    testMajorAxis<IMATH_INTERNAL_NAMESPACE::V4s>("V4s");
    testMajorAxis<IMATH_INTERNAL_NAMESPACE::V4i>("V4i");
    testMajorAxis<IMATH_INTERNAL_NAMESPACE::V4f>("V4f");
    testMajorAxis<IMATH_INTERNAL_NAMESPACE::V4d>("V4d");

    cout << "ok\n" << endl;
}
