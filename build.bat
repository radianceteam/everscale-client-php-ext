@echo off

rem This script requires min 2 arguments - PHP version and platform.
if "%1" == "" goto:usage
if "%2" == "" goto:usage

set PROJECT_NAME=TON SDK PHP extension
set ROOT_DIR=%cd%
set BUILD_DIR=%ROOT_DIR%\build
set PHP_VERSION=%1
set PLATFORM=%2
set ZTS=%3

echo Building %PROJECT_NAME% (PHP %PHP_VERSION%; PLATFORM %PLATFORM%; %ZTS%)

if not exist %BUILD_DIR% git clone https://github.com/microsoft/php-sdk-binary-tools.git %BUILD_DIR%
if not "%errorlevel%" == "0" exit /b %errorlevel%

cd %BUILD_DIR%
phpsdk-vc15-%PLATFORM%.bat -t %ROOT_DIR%\platform.bat --task-args "%PHP_VERSION% %ZTS%" && echo Done building %PROJECT_NAME% && cd %ROOT_DIR%
if not "%errorlevel%" == "0" exit /b %errorlevel%

:usage
echo usage: build.bat ^<PHP_VERSION^> ^<PLATFORM^> [^<ZTS^>]
echo example: build.bat 7.4.1 x86
echo example: build.bat 7.4.1 x64 ZTS
exit 1
