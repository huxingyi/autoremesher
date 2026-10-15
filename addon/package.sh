#!/bin/sh
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

# Build the installable add-on zip and the remesher CLI it needs, then print
# both paths. Run from anywhere:
#
#     sh addon/package.sh
#
# The CLI is built only if build/app/autoremesher is missing or lacks
# --stdin-control, with nice -n 10 make -j2 and no LTO (the laptop runs hot),
# and not while another build is running.
set -eu

repo=$(cd "$(dirname "$0")/.." && pwd)
package_dir="$repo/addon/autoremesher_blender"
build_dir="$repo/build"
cli="$build_dir/app/autoremesher"

if ! QT_QPA_PLATFORM=offscreen "$cli" --help 2>/dev/null | grep -q -- --stdin-control; then
    if pgrep -a 'cc1plus|lto1|make' >/dev/null; then
        echo "Another build is running; not starting the CLI build now:" >&2
        pgrep -a 'cc1plus|lto1|make' >&2
        exit 1
    fi
    echo "Building the CLI in $build_dir/app (nice -n 10 make -j2, no LTO)..."
    mkdir -p "$build_dir/app"
    (cd "$build_dir/app" \
        && qmake "$repo/autoremesher.pro" -after "QMAKE_CXXFLAGS_RELEASE-=-flto" "QMAKE_LFLAGS_RELEASE-=-flto" >/dev/null \
        && nice -n 10 make -j2 >make.log 2>&1) || {
        echo "CLI build failed; see $build_dir/app/make.log" >&2
        exit 1
    }
fi

name=$(python3 -c "import tomllib, sys; m = tomllib.load(open(sys.argv[1], 'rb')); print(m['id'] + '-' + m['version'])" \
    "$package_dir/blender_manifest.toml")
zip_path="$build_dir/addon/$name.zip"
mkdir -p "$build_dir/addon"
rm -f "$zip_path"
# Blender wants blender_manifest.toml at the root of the zip.
(cd "$package_dir" && zip -q -X "$zip_path" blender_manifest.toml ./*.py)

echo
echo "Add-on zip:      $zip_path"
echo "Remesher binary: $cli"
