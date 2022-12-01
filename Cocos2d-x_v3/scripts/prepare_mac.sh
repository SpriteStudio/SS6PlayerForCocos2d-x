#!/bin/bash -xe

SCRIPTDIR=$(dirname $0)
SCRIPTDIR=$(cd $SCRIPTDIR && pwd -P)
BASEDIR=${SCRIPTDIR}/..
BASEDIR=$(cd ${BASEDIR} && pwd -P)
ROOTDIR=${BASEDIR}/..
ROOTDIR=$(cd ${ROOTDIR} && pwd -P)

pushd ${ROOTDIR}/cocos2d > /dev/null
# checkout cocos2d-x v3 branch head and pull
if type git &>/dev/null; then
  git checkout v3
  git pull origin v3
  git checkout .
  git clean -fdx .
fi

# download cocos2d-x depends on 3rd party libraries
MAJOR_VER=$(sw_vers -productVersion | cut -d'.' -f1)
if [ "$MAJOR_VER" -ge "12" ]; then
    git checkout ./download-deps.py
    sed -i.bak -e 's|#!/usr/bin/env python|#!/usr/bin/env python3|g' ./download-deps.py
fi
./download-deps.py -r yes
popd > /dev/null # ${ROOTDIR}/cocos2d

pushd ${BASEDIR}/samples
/bin/rm -f cocos2d*
/bin/ln -s ../../cocos2d ./cocos2d

pushd Classes
/bin/rm -f SSPlayer*
/bin/ln -s ../../SSPlayer ./SSPlayer
popd > /dev/null # Classes

popd > /dev/null # ${BASEDIR}/samples
