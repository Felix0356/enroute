#!/bin/bash

#
# This script builds "enroute flight navigation" for Android in debug mode.
#
# See https://github.com/Akaflieg-Freiburg/enroute/wiki/Build-scripts
#

#
# Copyright © 2020 Stefan Kebekus <stefan.kebekus@math.uni-freiburg.de>
#
# This program is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License as published by the Free Software
# Foundation; either version 3 of the License, or (at your option) any later
# version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License along with
# this program; if not, write to the Free Software Foundation, Inc., 59 Temple
# Place - Suite 330, Boston, MA 02111-1307, USA.
#


#
# Fail on first error
#

set -e

#
# Clean up
#

rm -rf build-android-debug
mkdir -p build-android-debug
cd build-android-debug

#
# Configure
#


export ANDROID_NDK_ROOT=$ANDROID_SDK_ROOT/ndk/23.1.7779620
export JAVA_HOME=/usr/lib/jvm/java-17-openjdk-17.0.5.0.8-1.fc37.x86_64

# ~/Software/buildsystems/Qt/6.4.0/android_armv7/bin/qt-cmake .. -GNinja
$Qt6_DIR_ANDROID\_x86_64/bin/qt-cmake .. \
      -DQT_ANDROID_ABIS="arm64-v8a" \
      -G Ninja \
      -DCMAKE_BUILD_TYPE:STRING=Debug \
      -DOPENSSL_ROOT_DIR:PATH=$OPENSSL_ROOT_DIR

# Work around a bug in CMake Script…
#sed -i s/zipalign/31/ src/android-addhoursandminutes-deployment-settings.json


#
# Build the executable
#

ninja
