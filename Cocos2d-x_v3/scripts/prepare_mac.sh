#!/bin/bash -e

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
  git apply ${SCRIPTDIR}/xcode12.patch
fi

# download cocos2d-x depends on 3rd party libraries
./download-deps.py -r yes
popd > /dev/null # ${ROOTDIR}/cocos2d

pushd ${BASEDIR} > /dev/null
# re-create cocos2d-x directory smbolic-link
/bin/rm -f ./samples/cocos2d
ln -sfn ${ROOTDIR}/cocos2d ./samples/cocos2d

# re-create SSPlayer directory symbolic-link to Sample project
/bin/rm -f ./samples/Classes/SSPlayer
ln -sfn $(pwd)/SSPlayer ./samples/Classes/SSPlayer
popd > /dev/null # ${BASEDIR}
