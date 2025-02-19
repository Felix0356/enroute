#!/bin/bash

#
# This script builds "enroute flight navigation" for Android in release mode.
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

rm -rf build-android-release
mkdir -p build-android-release
cd build-android-release

#
# Configure
#

export ANDROID_NDK_ROOT=$ANDROID_SDK_ROOT/ndk/23.1.7779620
export JAVA_HOME=/usr/lib/jvm/java-17-openjdk-17.0.5.0.8-1.fc37.x86_64

$Qt6_DIR_ANDROID\_x86_64/bin/qt-cmake .. \
      -DQT_ANDROID_BUILD_ALL_ABIS:BOOL=On \
      -G Ninja \
      -DCMAKE_BUILD_TYPE:STRING=Debug \
      -DOPENSSL_ROOT_DIR:PATH=$OPENSSL_ROOT_DIR

#
# Build the executable
#

ninja

echo "Build AAB"
ninja aab


#
# Sign files … and done.
#

if [ -z "$ANDROID_KEYSTORE_FILE" -o -z "$ANDROID_KEYSTORE_PASS" ]
then
    echo "Not signing APK because \$ANDROID_KEYSTORE_FILE or \$ANDROID_KEYSTORE_PASS not set"
else
    echo "Signing APK"
    $ANDROID_SDK_ROOT/build-tools/33.0.1/apksigner sign \
						   --ks $ANDROID_KEYSTORE_FILE \
						   --ks-pass pass:$ANDROID_KEYSTORE_PASS \
						   --in src/android-build/build/outputs/apk/debug/android-build-debug.apk \
						   --out enroute-release-signed.apk
    echo "Signed APK file is available at $PWD/enroute-release-signed.apk"
    echo
    echo "Signing AAB"
    jarsigner -keystore $ANDROID_KEYSTORE_FILE \
	      -storepass $ANDROID_KEYSTORE_PASS \
	      src/android-build/build/outputs/bundle/release/android-build-release.aab "Stefan Kebekus"
    cp src/android-build/build/outputs/bundle/release/android-build-release.aab enroute-release-signed.aab
    
    echo "Signed AAB file is available at $PWD/enroute-release-signed.aab"
    nautilus $PWD &
fi
