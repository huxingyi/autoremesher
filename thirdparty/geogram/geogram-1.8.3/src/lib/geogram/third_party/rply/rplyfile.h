#ifndef RPLY_FILE_H
#define RPLY_FILE_H
/* ----------------------------------------------------------------------
 * RPly library, read/write PLY files
 * Diego Nehab, IMPA
 * http://www.impa.br/~diego/software/rply
 *
 * This library is distributed under the MIT License. See notice
 * at the end of this file.
 * ---------------------------------------------------------------------- */

#ifdef __cplusplus
extern "C" {
#endif

/* ----------------------------------------------------------------------
 * Opens a PLY file for reading (fails if file is not a PLY file)
 *
 * file_pointer: FILE * to file open for reading
 * error_cb: error callback function
 * idata,pdata: contextual information available to users
 *
 * Returns 1 if successful, 0 otherwise
 * ---------------------------------------------------------------------- */
p_ply ply_open_from_file(FILE *file_pointer, p_ply_error_cb error_cb,
    long idata, void *pdata);

/* ----------------------------------------------------------------------
 * Creates new PLY file
 *
 * file_pointer: FILE * to a file open for writing
 * storage_mode: file format mode
 * error_cb: error callback function
 * idata,pdata: contextual information available to users
 *
 * Returns handle to PLY file if successfull, NULL otherwise
 * ---------------------------------------------------------------------- */
p_ply ply_create_to_file(FILE *file_pointer, e_ply_storage_mode storage_mode,
        p_ply_error_cb error_cb, long idata, void *pdata);

#ifdef __cplusplus
}
#endif

#endif /* RPLY_FILE_H */

/* ----------------------------------------------------------------------
 * Copyright (C) 2003-2015 Diego Nehab. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 * ---------------------------------------------------------------------- */
