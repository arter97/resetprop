#!/bin/bash

API=29

aarch64-linux-android${API}-clang++ \
    -Iinclude \
        main.cpp \
        system_property_set.cpp \
        systemproperties/*.cpp \
    -s -Os -o resetprop && ls -al resetprop

# We don't have to worry about inconsistent C++ library
sed -i -e 's@libc++_shared.so@libc++.so\x00\x00\x00\x00\x00\x00\x00@g' resetprop
