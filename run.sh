#!/bin/bash

BUILD_DIR="build"

# Build dir if it doesn't exist
if [ ! -d "$BUILD_DIR" ]; then
    meson setup $BUILD_DIR
fi

# Looks like meson caches it, so it is safe to set every time
meson configure build -Dcompile_tests=false

case $1 in
    "wipe")
        echo "Wipe"
        meson --wipe build        
        ;;
    "release")
        echo "Release Build"        
        meson configure build -Ddebug=false
        meson setup --optimization=3 $BUILD_DIR
        meson compile -C $BUILD_DIR -v 
        ;;
    "test")
        echo "Test"
        meson configure build -Dcompile_tests=true
        meson setup --optimization=0 $BUILD_DIR
        meson test -C build --print-errorlogs --verbose  # --wrap='valgrind'
        exit 0
        ;;    
    *)
        # ninja -C $BUILD_DIR --optimization=g -v
        meson configure build -Ddebug=true
        meson setup --optimization=g $BUILD_DIR
        meson compile -C $BUILD_DIR -v
        ;;
esac



COMPRET=$?

case $1 in
    "build")
        exit 0
        ;;    
    *)
esac


if [ $COMPRET -eq 0 ]; then
    if [ $# -eq 0 ]; then        
        $BUILD_DIR/prog    
        exit 0
    fi
    
    case $1 in
        "release")
            $BUILD_DIR/prog    
            ;;
        "val")
            echo "Using valgrind."
            valgrind -s --leak-check=yes --tool=memcheck $BUILD_DIR/prog debug
            ;;    
        "debug")
            $BUILD_DIR/prog debug
            ;;
        *)
    esac
else
    echo "Compilation failed."
fi