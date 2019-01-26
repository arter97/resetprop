#!/bin/bash

aarch64-linux-android-clang++ \
    -Iinclude \
        main.cpp \
        system_property_set.cpp \
        systemproperties/*.cpp \
    -static -s -Os -o keymastermod \
    -static-libstdc++ && ls -al keymastermod
