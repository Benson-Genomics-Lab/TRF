#!/bin/bash
# Simple build script for TRF - no dependencies required
# Just needs gcc (or clang) and standard C library

set -e  # Exit on error

# Detect platform
UNAME_S=$(uname -s)
UNAME_M=$(uname -m)

case "$UNAME_S" in
    Linux*)
        PLATFORM="linux"
        if [ "$UNAME_M" = "x86_64" ]; then
            TARGET="linux64"
        else
            TARGET="linux32"
        fi
        ;;
    Darwin*)
        PLATFORM="mac"
        TARGET="mac"
        ;;
    CYGWIN*|MINGW*)
        PLATFORM="windows"
        if [ "$UNAME_M" = "x86_64" ]; then
            TARGET="dos64"
        else
            TARGET="dos32"
        fi
        ;;
    *)
        echo "Unknown platform: $UNAME_S"
        exit 1
        ;;
esac

echo "Building TRF for $PLATFORM ($TARGET)..."

# Compile flags
# Note: -fno-align-loops removed as it's not supported by clang
CFLAGS="-O2 -Wall -fno-align-functions"
LDFLAGS="-lm"

# Use CC environment variable if set, otherwise default to gcc
CC="${CC:-gcc}"

# Build
$CC $CFLAGS -DUNIXCONSOLE src/trf.c -o trf $LDFLAGS

echo "Build complete: ./trf"
echo "Test with: ./trf -v"
