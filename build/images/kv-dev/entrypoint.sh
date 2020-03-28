#!/bin/bash
#
# Copyright 2020, all rights reserved.
#

set -e

# first arg is option started with -/--
if [ "${1#-}" != "${1}" ];
then
    set -- "$@"
fi

if [ "${1}" != "bash" -a "$(id -u)" = '0' ];
then
    exec gosu ${KVDEV__USR} "$0" "$@"
fi

exec "$@"
