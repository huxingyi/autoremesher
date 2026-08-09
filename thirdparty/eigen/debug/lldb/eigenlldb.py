# -*- coding: utf-8 -*-
# This file is part of Eigen, a lightweight C++ template library
# for linear algebra.
#
# Copyright (C) 2021 Huang, Zhaoquan <zhaoquan2008@hotmail.com>
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# Pretty printers for Eigen::Matrix to use with LLDB debugger
#
# Usage:
# 1. Add the following line (change it according to the path to this file)
#    to the file ~/.lldbinit (create one if it doesn't exist):
#        `command script import /path/to/eigenlldb.py`
# 2. Inspect the variables in LLDB command line
#        `frame variable`

import lldb
from typing import List
import bisect


def __lldb_init_module(debugger, internal_dict):
    debugger.HandleCommand("type synthetic add -x Eigen::Matrix<.*> --python-class eigenlldb.EigenMatrixChildProvider")
    debugger.HandleCommand(
        "type synthetic add -x Eigen::SparseMatrix<.*> --python-class eigenlldb.EigenSparseMatrixChildProvider")


class EigenMatrixChildProvider:
    _valobj: lldb.SBValue
    _scalar_type: lldb.SBType
    _scalar_size: int
    _rows_compile_time: int
    _cols_compile_time: int
    _row_major: bool
    _fixed_storage: bool

    def __init__(self, valobj, internal_dict):
        self._valobj = valobj
        valtype = valobj.GetType().GetCanonicalType()

        scalar_type = valtype.GetTemplateArgumentType(0)
        if not scalar_type.IsValid():
            # In the case that scalar_type is invalid on LLDB 9.0 on Windows with CLion
            storage = valobj.GetChildMemberWithName("m_storage")
            data = storage.GetChildMemberWithName("m_data")
            data_type = data.GetType()
            if data_type.IsPointerType():
                scalar_type = data.GetType().GetPointeeType()
            else:
                scalar_type = data.GetChildMemberWithName("array").GetType().GetArrayElementType()
        self._scalar_type = scalar_type
        self._scalar_size = self._scalar_type.GetByteSize()

        name = valtype.GetName()
        template_begin = name.find("<")
        template_end = name.find(">")
        template_args = name[(template_begin + 1):template_end].split(",")
        self._rows_compile_time = int(template_args[1])
        self._cols_compile_time = int(template_args[2])
        self._row_major = (int(template_args[3]) & 1) != 0

        max_rows = int(template_args[4])
        max_cols = int(template_args[5])
        self._fixed_storage = (max_rows != -1 and max_cols != -1)

    def num_children(self):
        return self._cols() * self._rows()

    def get_child_index(self, name):
        pass

    def get_child_at_index(self, index):
        storage = self._valobj.GetChildMemberWithName("m_storage")
        data = storage.GetChildMemberWithName("m_data")
        offset = self._scalar_size * index

        if self._row_major:
            row = index // self._cols()
            col = index % self._cols()
        else:
            row = index % self._rows()
            col = index // self._rows()
        if self._fixed_storage:
            data = data.GetChildMemberWithName("array")
        if self._cols() == 1:
            name = '[{}]'.format(row)
        elif self._rows() == 1:
            name = '[{}]'.format(col)
        else:
            name = '[{},{}]'.format(row, col)
        return data.CreateChildAtOffset(
            name, offset, self._scalar_type
        )

    def _cols(self):
        if self._cols_compile_time == -1:
            storage = self._valobj.GetChildMemberWithName("m_storage")
            cols = storage.GetChildMemberWithName("m_cols")
            return cols.GetValueAsUnsigned()
        else:
            return self._cols_compile_time

    def _rows(self):
        if self._rows_compile_time == -1:
            storage = self._valobj.GetChildMemberWithName("m_storage")
            rows = storage.GetChildMemberWithName("m_rows")
            return rows.GetValueAsUnsigned()
        else:
            return self._rows_compile_time


class EigenSparseMatrixChildProvider:
    _valobj: lldb.SBValue
    _scalar_type: lldb.SBType
    _scalar_size: int
    _index_type: lldb.SBType
    _index_size: int
    _row_major: bool

    _outer_size: int
    _nnz: int
    _values: lldb.SBValue
    _inner_indices: lldb.SBValue
    _outer_starts: lldb.SBValue
    _inner_nnzs: lldb.SBValue
    _compressed: bool

    # Index of the first synthetic child under each outer index
    _child_indices: List[int]

    def __init__(self, valobj, internal_dict):
        self._valobj = valobj
        valtype = valobj.GetType().GetCanonicalType()
        scalar_type = valtype.GetTemplateArgumentType(0)
        if not scalar_type.IsValid():
            # In the case that scalar_type is invalid on LLDB 9.0 on Windows with CLion
            data = valobj.GetChildMemberWithName("m_data")
            values = data.GetChildMemberWithName("m_values")
            scalar_type = values.GetType().GetPointeeType()
        self._scalar_type = scalar_type
        self._scalar_size = scalar_type.GetByteSize()

        index_type = valtype.GetTemplateArgumentType(2)
        if not index_type.IsValid():
            # In the case that scalar_type is invalid on LLDB 9.0 on Windows with CLion
            outer_starts = valobj.GetChildMemberWithName("m_outerIndex")
            index_type = outer_starts.GetType().GetPointeeType()
        self._index_type = index_type
        self._index_size = index_type.GetByteSize()

        name = valtype.GetName()
        template_begin = name.find("<")
        template_end = name.find(">")
        template_args = name[(template_begin + 1):template_end].split(",")
        self._row_major = (int(template_args[1]) & 1) != 0

    def num_children(self):
        return self._nnz + 2

    def get_child_index(self, name):
        pass

    def get_child_at_index(self, index):
        if index == 0:
            name = "rows" if self._row_major else "cols"
            return self._valobj.GetChildMemberWithName("m_outerSize") \
                .CreateChildAtOffset(name, 0, self._index_type)
        elif index == 1:
            name = "cols" if self._row_major else "rows"
            return self._valobj.GetChildMemberWithName("m_innerSize") \
                .CreateChildAtOffset(name, 0, self._index_type)
        else:
            index = index - 2
        outer_index = bisect.bisect_right(self._child_indices, index) - 1
        total_nnzs = self._child_indices[outer_index]
        if self._compressed:
            item_index = index
            inner_index = self._inner_indices \
                .CreateChildAtOffset("", item_index * self._index_size, self._index_type) \
                .GetValueAsUnsigned()
            return self._values \
                .CreateChildAtOffset(self._child_name(outer_index, inner_index),
                                     item_index * self._scalar_size,
                                     self._scalar_type)
        else:
            index_begin = self._outer_starts \
                .CreateChildAtOffset("", outer_index * self._index_size, self._index_type) \
                .GetValueAsUnsigned()
            item_index = index - total_nnzs + index_begin
            inner_index = self._inner_indices \
                .CreateChildAtOffset("", item_index * self._index_size, self._index_type) \
                .GetValueAsUnsigned()
            return self._values \
                .CreateChildAtOffset(self._child_name(outer_index, inner_index),
                                     item_index * self._scalar_size,
                                     self._scalar_type)

    def update(self):
        valobj = self._valobj
        self._outer_size = valobj.GetChildMemberWithName("m_outerSize").GetValueAsUnsigned()
        data = valobj.GetChildMemberWithName("m_data")
        self._values = data.GetChildMemberWithName("m_values")
        self._inner_indices = data.GetChildMemberWithName("m_indices")
        self._outer_starts = valobj.GetChildMemberWithName("m_outerIndex")
        self._inner_nnzs = valobj.GetChildMemberWithName("m_innerNonZeros")

        self._compressed = self._inner_nnzs.GetValueAsUnsigned() == 0

        total_nnzs = 0
        child_indices = [0]
        for outer_index in range(self._outer_size):
            if self._compressed:
                index_end = self._outer_starts \
                    .CreateChildAtOffset("", (outer_index + 1) * self._index_size, self._index_type) \
                    .GetValueAsUnsigned()
                total_nnzs = index_end
                child_indices.append(total_nnzs)
            else:
                nnzs = self._inner_nnzs \
                    .CreateChildAtOffset("", outer_index * self._index_size, self._index_type) \
                    .GetValueAsUnsigned()
                total_nnzs = total_nnzs + nnzs
                child_indices.append(total_nnzs)
        self._child_indices = child_indices
        self._nnz = total_nnzs

    def _child_name(self, outer_index, inner_index):
        if self._row_major:
            return "[{0},{1}]".format(outer_index, inner_index)
        else:
            return "[{1},{0}]".format(outer_index, inner_index)
