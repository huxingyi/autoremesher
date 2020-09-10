// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the OpenEXR Project.

#pragma once

#ifndef INCLUDE_TestUtilFStream_h_
#define INCLUDE_TestUtilFStream_h_ 1

#include <fstream>
#include <string>

#ifdef _WIN32
# define VC_EXTRALEAN
# include <string.h>
# include <windows.h>
# include <io.h>
# include <fcntl.h>
# include <sys/types.h>
# include <sys/stat.h>
# include <share.h>
#endif

namespace testutil
{
#ifdef _WIN32
inline std::wstring
WidenFilename (const char* filename)
{
    std::wstring ret;
    int          fnlen = static_cast<int> (strlen (filename));
    int len = MultiByteToWideChar (CP_UTF8, 0, filename, fnlen, NULL, 0);
    if (len > 0)
    {
        ret.resize (len);
        MultiByteToWideChar (CP_UTF8, 0, filename, fnlen, &ret[0], len);
    }
    return ret;
}

// This is a big work around mechanism for compiling using mingw / gcc under windows
// until mingw 9 where they add the wide filename version of open
# if (defined(__GLIBCXX__) && !(defined(_GLIBCXX_HAVE_WFOPEN) && defined(_GLIBCXX_USE_WCHAR_T)))
#  define USE_WIDEN_FILEBUF 1
template <typename CharT, typename TraitsT>
class WidenFilebuf : public std::basic_filebuf<CharT, TraitsT>
{
    inline int mode_to_flags (std::ios_base::openmode mode)
    {
        int flags = 0;
        if (mode & std::ios_base::in) flags |= _O_RDONLY;
        if (mode & std::ios_base::out)
        {
            flags |= _O_WRONLY;
            flags |= _O_CREAT;
            if (mode & std::ios_base::app) flags |= _O_APPEND;
            if (mode & std::ios_base::trunc) flags |= _O_TRUNC;
        }
        if (mode & std::ios_base::binary)
            flags |= _O_BINARY;
        else
            flags |= _O_TEXT;
        return flags;
    }

public:
    using base_filebuf = std::basic_filebuf<CharT, TraitsT>;
    inline base_filebuf* wide_open (std::wstring& fn, std::ios_base::openmode m)
    {
        if (this->is_open () || fn.empty ())
            return nullptr;

        int     fd;
        errno_t e = _wsopen_s (
            &fd,
            fn.c_str (),
            mode_to_flags (m),
            _SH_DENYNO,
            _S_IREAD | _S_IWRITE);
        if (e != 0)
            return nullptr;

        // sys_open will do an fdopen internally which will then clean up the fd upon close
        this->_M_file.sys_open (fd, m);
        if (this->is_open ())
        {
            // reset the internal state, these members are consistent between gcc versions 4.3 - 9
            // but at 9, the wfopen stuff should become available, such that this will no longer be
            // active
            this->_M_allocate_internal_buffer ();
            this->_M_mode    = m;
            this->_M_reading = false;
            this->_M_writing = false;
            this->_M_set_buffer (-1);
            this->_M_state_last = this->_M_state_cur = this->_M_state_beg;

            if ((m & std::ios_base::ate) &&
                this->seekoff (0, std::ios_base::end, m) ==
                    static_cast<typename base_filebuf::pos_type> (-1))
            {
                this->close ();
                return nullptr;
            }
        }
        return this;
    }
};
# endif // __GLIBCXX__
#endif     // _WIN32

template <typename StreamType>
inline void
OpenStreamWithUTF8Name (
    StreamType& is, const char* filename, std::ios_base::openmode mode)
{
#ifdef _WIN32
    std::wstring wfn = WidenFilename (filename);
# ifdef USE_WIDEN_FILEBUF
    using CharT   = typename StreamType::char_type;
    using TraitsT = typename StreamType::traits_type;
    using wbuf    = WidenFilebuf<CharT, TraitsT>;
    if (!static_cast<wbuf*> (is.rdbuf ())->wide_open (wfn, mode))
        is.setstate (std::ios_base::failbit);
    else
        is.clear ();
# else
    is.rdbuf ()->open (wfn.c_str (), mode);
# endif
#else
    is.rdbuf ()->open (filename, mode);
#endif
}

} // namespace testutil

#endif // INCLUDE_TestUtilFStream_h_
