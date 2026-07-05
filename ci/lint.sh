#!/bin/bash
# Run clang-format on the project source files.
# Uses .clang-format from the project root.
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"
find "$PROJECT_DIR/src" -iname "*.h" -o -iname "*.cpp" | xargs clang-format -style=file -i