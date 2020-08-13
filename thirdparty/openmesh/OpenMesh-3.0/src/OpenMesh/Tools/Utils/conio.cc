/*===========================================================================*\
 *                                                                           *
 *                               OpenMesh                                    *
 *      Copyright (C) 2001-2014 by Computer Graphics Group, RWTH Aachen      *
 *                           www.openmesh.org                                *
 *                                                                           *
 *---------------------------------------------------------------------------* 
 *  This file is part of OpenMesh.                                           *
 *                                                                           *
 *  OpenMesh is free software: you can redistribute it and/or modify         * 
 *  it under the terms of the GNU Lesser General Public License as           *
 *  published by the Free Software Foundation, either version 3 of           *
 *  the License, or (at your option) any later version with the              *
 *  following exceptions:                                                    *
 *                                                                           *
 *  If other files instantiate templates or use macros                       *
 *  or inline functions from this file, or you compile this file and         *
 *  link it with other files to produce an executable, this file does        *
 *  not by itself cause the resulting executable to be covered by the        *
 *  GNU Lesser General Public License. This exception does not however       *
 *  invalidate any other reasons why the executable file might be            *
 *  covered by the GNU Lesser General Public License.                        *
 *                                                                           *
 *  OpenMesh is distributed in the hope that it will be useful,              *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 *  GNU Lesser General Public License for more details.                      *
 *                                                                           *
 *  You should have received a copy of the GNU LesserGeneral Public          *
 *  License along with OpenMesh.  If not,                                    *
 *  see <http://www.gnu.org/licenses/>.                                      *
 *                                                                           *
\*===========================================================================*/ 

/*===========================================================================*\
 *                                                                           *             
 *   $Revision$                                                         *
 *   $Date$                   *
 *                                                                           *
\*===========================================================================*/

#include <OpenMesh/Core/System/config.hh>
#include <OpenMesh/Tools/Utils/conio.hh>

// ----------------------------------------------------------------- MSVC Compiler ----
#ifdef _MSC_VER

#include <conio.h>

namespace OpenMesh {
namespace Utils {

int kbhit()  { return ::_kbhit();  }
int getch()  { return ::_getch();  }
int getche() { return ::_getche(); }

} // Tools
} // AS

// ----------------------------------------------------------------- Win32 ----
#elif defined(WIN32)

#include <conio.h>

namespace OpenMesh {
namespace Utils {

int kbhit()  { return ::kbhit();  }
int getch()  { return ::getch();  }
int getche() { return ::getche(); }

} // Tools
} // AS
// ----------------------------------------------------------------- Other ----
#else

// Based on code published by Floyd Davidson in a newsgroup.

#include <stdio.h>     /* stdout, fflush() */
#if !defined(POSIX_1003_1_2001)
#  include <fcntl.h>
#  include <unistd.h>  
#else
#  include <select.h>  /* select()       */
#endif
#include <termios.h>   /* tcsetattr()    */
#include <sys/ioctl.h> /* ioctl()        */
#include <sys/time.h>  /* struct timeval */

namespace OpenMesh {
namespace Utils {

#ifdef CTIME
#  undef CTIME
#endif
#define CTIME 1
#define CMIN  1


int kbhit(void)
{
  int cnt = 0;
  int error;
  static struct termios Otty, Ntty;

  tcgetattr(0, &Otty);
  Ntty = Otty;

  Ntty.c_iflag      = 0; /* input mode */
  Ntty.c_oflag      = 0; /* output mode */
  Ntty.c_lflag     &= ~ICANON; /* raw mode */
  Ntty.c_cc[VMIN]   = CMIN; /* minimum chars to wait for */
  Ntty.c_cc[VTIME]  = CTIME; /* minimum wait time */

  if (0 == (error = tcsetattr(0, TCSANOW, &Ntty))) 
  {
    struct timeval tv;
    error += ioctl(0, FIONREAD, &cnt);
    error += tcsetattr(0, TCSANOW, &Otty);
    tv.tv_sec = 0;
    tv.tv_usec = 100; /* insert at least a minimal delay */
    select(1, NULL, NULL, NULL, &tv);
  }
  return (error == 0 ? cnt : -1 );
}


int getch(void)
{
  char ch = ' ';
  int error;
  static struct termios Otty, Ntty;
  
  fflush(stdout);
  tcgetattr(0, &Otty);
  Ntty = Otty;
  
  Ntty.c_iflag     = 0;        // input mode
  Ntty.c_oflag     = 0;        // output mode
  Ntty.c_lflag    &= ~ICANON;  // line settings  
  Ntty.c_lflag    &= ~ECHO;    // enable echo
  Ntty.c_cc[VMIN]  = CMIN;     // minimum chars to wait for
  Ntty.c_cc[VTIME] = CTIME;    // minimum wait time

  // Conditionals allow compiling with or without flushing pre-existing
  // existing buffered input before blocking.
#if 1
  // use this to flush the input buffer before blocking for new input
#  define FLAG TCSAFLUSH
#else
  // use this to return a char from the current input buffer, or block if
  // no input is waiting.
#  define FLAG TCSANOW
#endif

  if (0 == (error = tcsetattr(0, FLAG, &Ntty))) 
  {
    error = read(0, &ch, 1 );           // get char from stdin
    error += tcsetattr(0, FLAG, &Otty); // restore old settings
  }
  return (error == 1 ? (int) ch : -1 );
}


int getche(void)
{
  char ch = ' ';
  int error;
  static struct termios Otty, Ntty;
  
  fflush(stdout);
  tcgetattr(0, &Otty);
  Ntty = Otty;
  
  Ntty.c_iflag     = 0;        // input mode
  Ntty.c_oflag     = 0;        // output mode
  Ntty.c_lflag    &= ~ICANON;  // line settings  
  Ntty.c_lflag    |= ECHO;     // enable echo
  Ntty.c_cc[VMIN]  = CMIN;     // minimum chars to wait for
  Ntty.c_cc[VTIME] = CTIME;    // minimum wait time

  // Conditionals allow compiling with or without flushing pre-existing
  // existing buffered input before blocking.
#if 1
  // use this to flush the input buffer before blocking for new input
#  define FLAG TCSAFLUSH
#else
  // use this to return a char from the current input buffer, or block if
  // no input is waiting.
#  define FLAG TCSANOW
#endif

  if (0 == (error = tcsetattr(0, FLAG, &Ntty))) {
    error = read(0, &ch, 1 );           // get char from stdin
    error += tcsetattr(0, FLAG, &Otty); // restore old settings
  }

  return (error == 1 ? (int) ch : -1 );
}

} // namespace Tools
} // namespace AS
// ----------------------------------------------------------------------------
#endif // System dependent parts
// ============================================================================

//#define Test
#if defined(Test)

#include <ctype.h>

int main (void) 
{ 
   char  msg[] = "press key to continue...";
   char *ptr   = msg;

  while ( !OpenMesh::Utils::kbhit() )
  {
    char* tmp = *ptr;
    *ptr = islower(tmp) ? toupper(tmp) : tolower(tmp);
    printf("\r%s", msg); fflush(stdout);
    *ptr = (char)tmp;
    if (!*(++ptr)) 
      ptr = msg; 
    usleep(20000);
  }

  printf("\r%s.", msg); fflush(stdout);
  OpenMesh::Utils::getch();
  printf("\r%s..", msg); fflush(stdout);
  OpenMesh::Utils::getche();
  return 0;
}

#endif // Test

// ============================================================================
