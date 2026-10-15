# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 AutoRemesher fork contributors
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

"""The add-on's name, in one place (working name; may change before release).

To rename: change these two and `id` / `name` in blender_manifest.toml, which
Blender reads without importing Python (test_worker.py checks they agree).
The core and its CLI keep the name autoremesher.
"""

NAME = "Quadami"  # shown in the UI
ID = "quadami"    # operator ids, scene and object properties, extension id
