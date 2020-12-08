@echo off
setlocal
set SCRIPTDIR=%~dp0
set BASEDIR=%SCRIPTDIR%..
set ROOTDIR=%BASEDIR%\..
@echo on

pushd %ROOTDIR%\cocos2d
rem checkout cocos2d-x v3 branch head and pull
git checkout v3 || exit 1
git pull origin v3 || exit 1
git checkout . || exit 1
git clean -fdx . || exit 1

rem download cocos2d-x depends on 3rd party libraries (use python2)
python download-deps.py -r no || exit 1
popd 

pushd %BASEDIR%
rem re-create cocos2d-x directory smbolic-link
del /f samples\cocos2d
del /f samples\cocos2d.lnk
mklink /d samples\cocos2d %ROOTDIR%\cocos2d

rem re-create SSPlayer directory symbolic-link to Sample project
del /f samples\Classes\SSPlayer
del /f samples\Classes\SSPlayer.lnk
mklink /d samples\Classes\SSPlayer %BASEDIR%\SSPlayer
popd
