#!/bin/bash

# Format files with extensions, excluding plugins because they're partial files that don't contain valid syntax
find . -type f \( -name '*.c' -o -name '*.cc' -o -name '*.cpp' -o -name '*.cu' -o -name '*.cxx' -o -name '*.h' -o -name '*.inc' -not -path '*/plugins/*' \) | xargs -n 1 -P 0 clang-format-17 -i

# Format main headers without extensions
find Eigen -maxdepth 1 -type f | xargs -n 1 -P 0 clang-format-17 -i
find unsupported/Eigen -maxdepth 2 -type f -not -name '*.txt' | xargs -n 1 -P 0 clang-format-17 -i

# Format examples
find doc/examples -type f -name '*.cpp.*' | xargs -n 1 -P 0 clang-format-17 -i
