#!/bin/bash
VER_GREP=`grep APP_VERSION_STR res/resource.h`
VER_ARR=($VER_GREP)
VER_STR=`echo ${VER_ARR[2]} | sed "s/\"//g"`
MACVER_STR=`echo ${VER_STR} | tr . '\n' | awk '{ print $1}'`
MACVER_ARR=(${MACVER_STR})
if [[ "$1" == "--compat" ]]; then
    MAC_VER="${MACVER_ARR[0]}.${MACVER_ARR[1]}.0"
elif [[ "$1" == "--full" ]]; then
    MAC_VER="${MACVER_ARR[0]}.${MACVER_ARR[1]}.${MACVER_ARR[2]}.${MACVER_ARR[3]}"
else
    MAC_VER="${MACVER_ARR[0]}.${MACVER_ARR[1]}.${MACVER_ARR[2]}"
fi
echo ${MAC_VER}
