#!/bin/bash
#
# Copyright 2020, all rights reserved
#

_SELF_PATH=$(dirname $(readlink -f $0))

source "${_SELF_PATH}/mylog.sh"

function tailor_boost_build {
    if [ ! -f boost-build.jam ];then
        return 1
    fi

    sed -i 's/^[ ]*boost-build/#&/g' boost-build.jam
    echo "boost-build ${KVDEV__BOOST_BUILD_HOME}/src/kernel ;" >> boost-build.jam

    return 0
}


MYLOG__INFO "Start to build ..."

source "${_SELF_PATH}/setenv"

if [ ! -f "${KVDEV__BUILD_BIN}/boost.jam" ]; then
    MYLOG__FATAL "File not exist: [${KVDEV__BUILD_BIN}/boost.jam]!"
fi

if [ ! -d "${KVDEV__BOOST_BUILD_HOME}/src/tools" ]; then
    MYLOG__FATAL "Boost build directory not exist: [${KVDEV__BOOST_BUILD_HOME}/src/tools]!"
fi

if [ ! -f "${KVDEV__BUILD_BIN}/user-config.jam" ]; then
    MYLOG__FATAL "File not exist: [${KVDEV__BUILD_BIN}/user-config.jam]!"
fi

if [ ! -d "${KVDEV__PROJECT_HOME}" ]; then
    MYLOG__FATAL "Project home directory not exist: [${KVDEV__PROJECT_HOME}]!"
fi

if [ ! -d "${KVDEV__PROJECT_HOME}/src" ]; then
    MYLOG__FATAL "Project source directory not exist: [${KVDEV__PROJECT_HOME}/src]!"
fi

if [ ! -f "${KVDEV__PROJECT_HOME}/boost-build.jam" ]; then
    MYLOG__FATAL "File not exist: [${KVDEV__PROJECT_HOME}/boost-build.jam]!"
fi

MYLOG__INFO "Build environment check passed ..."

# Copy boost.jam into $BOOST_BUILD/src/tools.
cp -f "${KVDEV__BUILD_BIN}/boost.jam" "${KVDEV__BOOST_BUILD_HOME}/src/tools"

# Copy user-config.jam to home directory
cp -f "${KVDEV__BUILD_BIN}/user-config.jam" "${HOME}"

# Tailor boost-build.jam
cd ${KVDEV__PROJECT_HOME}
tailor_boost_build

MYLOG__INFO "Build setup done ..."

cd "${KVDEV__PROJECT_HOME}/src"
b2

MYLOG__INFO "Build completed!"

