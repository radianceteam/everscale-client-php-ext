#!/bin/bash

set -e

function usage() {
  echo <<<EOT
Usage: build.sh [args] [/path/to/sdk/installation/directory]
Args:
    -d      Enable debug output.
    -h      Show this help.
EOT
}

ENABLE_DEBUG=0

while getopts ":dh" opt; do
  case ${opt} in
    d )
      ENABLE_DEBUG=1
      ;;
    h )
      usage
      exit 0
      ;;
    \? )
      usage
      ;;
  esac
done

shift $(($OPTIND - 1))

SRC_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
BUILD_DIR=${SRC_DIR}/build
SDK_INSTALL_DIR=$HOME/ton-sdk

if [ "$#" -ne 0 ]; then
  SDK_INSTALL_DIR=$1
fi

CONFIGURE_OPTIONS=--with-ton_client=${SDK_INSTALL_DIR}
if [ "${ENABLE_DEBUG}" -ne 0 ]; then
  CONFIGURE_OPTIONS="${CONFIGURE_OPTIONS} --enable-ton_client_debug"
fi

rm -rf ${BUILD_DIR}
mkdir -p ${BUILD_DIR}
cp -r ${SRC_DIR}/src/* ${BUILD_DIR}

cd ${BUILD_DIR}
phpize
./configure ${CONFIGURE_OPTIONS}
make

#sudo make install
