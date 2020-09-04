/*
 *  Copyright (c) 2012-2014, Bruno Levy
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *  this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *  this list of conditions and the following disclaimer in the documentation
 *  and/or other materials provided with the distribution.
 *  * Neither the name of the ALICE Project-Team nor the names of its
 *  contributors may be used to endorse or promote products derived from this
 *  software without specific prior written permission.
 * 
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 *  If you modify this software, you should include a notice giving the
 *  name of the person performing the modification, the date of modification,
 *  and the reason for such modification.
 *
 *  Contact: Bruno Levy
 *
 *     Bruno.Levy@inria.fr
 *     http://www.loria.fr/~levy
 *
 *     ALICE Project
 *     LORIA, INRIA Lorraine, 
 *     Campus Scientifique, BP 239
 *     54506 VANDOEUVRE LES NANCY CEDEX 
 *     FRANCE
 *
 */

#include <geogram/delaunay/periodic.h>

namespace GEO {

    int Periodic::translation[27][3] = {
	{  0,  0,  0}, //13 -> 0   +   <-- zero displacement is first.
	{ -1, -1, -1}, //0  -> 1   -
	{ -1, -1,  0}, //1  -> 2   -
	{ -1, -1,  1}, //2  -> 3   -
	{ -1,  0, -1}, //3  -> 4   -
	{ -1,  0,  0}, //4  -> 5   - 
	{ -1,  0,  1}, //5  -> 6   -
	{ -1,  1, -1}, //6  -> 7   -
	{ -1,  1,  0}, //7  -> 8   -
	{ -1,  1,  1}, //8  -> 9   - 
	{  0, -1, -1}, //9  -> 10  -
	{  0, -1,  0}, //10 -> 11  -
	{  0, -1,  1}, //11 -> 12  -
	{  0,  0, -1}, //12 -> 13  -
	// (zero displacement was there)
	{  0,  0,  1}, //14 -> 14  +
	{  0,  1, -1}, //15 -> 15  -
	{  0,  1,  0}, //16 -> 16  +
	{  0,  1,  1}, //17 -> 17  +
	{  1, -1, -1}, //18 -> 18  -
	{  1, -1,  0}, //19 -> 19  -
	{  1, -1,  1}, //20 -> 20  -
	{  1,  0, -1}, //21 -> 21  -
	{  1,  0,  0}, //22 -> 22  +
	{  1,  0,  1}, //23 -> 23  +
	{  1,  1, -1}, //24 -> 24  -
	{  1,  1,  0}, //25 -> 25  +
	{  1,  1,  1}  //26 -> 26  +
    };

    int Periodic::reorder_instances[27] = {
	1 , 2 , 3 , 4 , 5 , 6 , 7 , 8 , 9 ,
	10, 11, 12, 13, 0 , 14, 15, 16, 17,
	18, 19, 20, 21, 22, 23, 24, 25, 26
    };

    bool Periodic::instance_is_positive[27] = {
	true,  false, false, false, false, false, 
	false, false, false, false, false, false, 
	false, false, true,  false, true,  true,  
	false, false, false, false, true,  true,
	false, true,  true
    };
}

