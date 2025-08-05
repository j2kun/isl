#!/bin/bash

# Regex to replace angle bracket includes with quotes for local ISL files
# This matches includes like: #include <isl_*.h> or #include <gitversion.h> etc.

find . -name "*.c" -o -name "*.h" | grep -v "./include/" | while read file; do
    echo "Processing $file"
    sed -i.bak -E 's|#include <(isl_[^>]+\.h)>|#include "\1"|g; s|#include <(gitversion\.h)>|#include "\1"|g' "$file"
done

# Also fix template includes
find . -name "*.c" -o -name "*.h" | grep -v "./include/" | while read file; do
    sed -i -E 's|#include <([^/>]*_templ\.[ch])>|#include "\1"|g' "$file"
done

echo "Include fixes applied!"