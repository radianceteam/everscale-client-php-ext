@echo off

rem This script requires min 1 argument - PHP version
if "%1" == "" goto:usage

rem This script is required to run in PHP SDK shell
if "%PHP_SDK_ARCH%" == "" goto:noenv
if "%PHP_SDK_VS%" == "" goto:noenv

set ROOT_DIR=%cd%\..
set BUILD_DIR=%cd%
set PLATFORM=%PHP_SDK_ARCH%
set PHP_VERSION=%1
set ZTS_ENABLED=0
set ZTS_SUFFIX=nts
set PHP_SRC_DIR=%BUILD_DIR%\phpmaster\%PHP_SDK_VS%\%PHP_SDK_ARCH%\php-src
set PHP_RELEASE_DIR_NAME=Release

if "%2" == "ZTS" set ZTS_ENABLED=1
if "%2" == "ZTS" set ZTS_SUFFIX=zts
if "%2" == "ZTS" set PHP_RELEASE_DIR_NAME=Release_TS

set RELEASE_DIR=%BUILD_DIR%\release\%PLATFORM%\%ZTS_SUFFIX%
set PHP_RELEASE_DIR=%PHP_SRC_DIR%\%PHP_RELEASE_DIR_NAME%
if "%PHP_SDK_ARCH%" == "x64" set PHP_RELEASE_DIR=%PHP_SRC_DIR%\x64\%PHP_RELEASE_DIR_NAME%

cmd /c "phpsdk_buildtree phpmaster"
cd %BUILD_DIR%\phpmaster\%PHP_SDK_VS%\%PHP_SDK_ARCH%
if not "%errorlevel%" == "0" exit /b %errorlevel%

if not exist php-src git clone https://github.com/php/php-src.git %PHP_SRC_DIR%
if not "%errorlevel%" == "0" exit /b %errorlevel%

cd %PHP_SRC_DIR%
git checkout php-%PHP_VERSION%
cmd /c phpsdk_deps --update --branch "%PHP_VERSION:~0,3%"%
if not "%errorlevel%" == "0" exit /b %errorlevel%

if not exist ext\ton_client mkdir %PHP_SRC_DIR%\ext\ton_client
if not "%errorlevel%" == "0" exit /b %errorlevel%

xcopy /Y /s %ROOT_DIR%\src %PHP_SRC_DIR%\ext\ton_client
if not "%errorlevel%" == "0" exit /b %errorlevel%

xcopy /Y /s %ROOT_DIR%\deps\bin\%PLATFORM%\* ..\deps\bin
if not "%errorlevel%" == "0" exit /b %errorlevel%

xcopy /Y /s %ROOT_DIR%\deps\lib\%PLATFORM%\* ..\deps\lib
if not "%errorlevel%" == "0" exit /b %errorlevel%

xcopy /Y /s %ROOT_DIR%\deps\include ..\deps\include
if not "%errorlevel%" == "0" exit /b %errorlevel%

cmd /c buildconf
if not "%errorlevel%" == "0" exit /b %errorlevel%

if "%ZTS_ENABLED%" == "0" set ZTS_FLAG="--disable-zts"
if "%ZTS_ENABLED%" == "1" set ZTS_FLAG="--enable-zts"
cmd /c "configure --disable-all --enable-cli --enable-json --enable-ton_client %ZTS_FLAG%"
if not "%errorlevel%" == "0" exit /b %errorlevel%

cmd /c nmake
if not "%errorlevel%" == "0" exit /b %errorlevel%

if not exist %RELEASE_DIR% mkdir %RELEASE_DIR%
if not "%errorlevel%" == "0" exit /b %errorlevel%

xcopy /Y /s %ROOT_DIR%\deps\bin\%PLATFORM%\*.dll %RELEASE_DIR%
if not "%errorlevel%" == "0" exit /b %errorlevel%

xcopy /Y /s %PHP_RELEASE_DIR%\php_ton_client.dll %RELEASE_DIR%
if not "%errorlevel%" == "0" exit /b %errorlevel%

exit 0

:usage
echo usage: platform.bat ^<php_version&> ^<ZTS^>
echo example: platform.bat 7.4.1
echo example: platform.bat 7.4.1 ZTS
exit 1

:noenv
echo This script should be run in PHP SDK shell.
exit 1
