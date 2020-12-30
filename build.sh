#!/bin/bash

set -e

SRC_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
BUILD_DIR=${SRC_DIR}/build
SDK_INSTALL_DIR=$HOME/ton-sdk

if [ "$#" -ne 0 ]; then
  SDK_INSTALL_DIR=$1
fi

rm -rf ${BUILD_DIR}
mkdir -p ${BUILD_DIR}
cp -r ${SRC_DIR}/src/* ${BUILD_DIR}

cd ${BUILD_DIR}
phpize
./configure --with-ton_client=${SDK_INSTALL_DIR}
make

#sudo make install
