@echo off

rem This script requires min 2 arguments - PHP version and platform.
if "%1" == "" goto:usage
if "%2" == "" goto:usage
if "%3" == "" goto:usage

set PROJECT_NAME=TON SDK PHP extension
set ROOT_DIR=%cd%
set BUILD_DIR=%ROOT_DIR%\build
set PHP_VERSION=%1
set VC_VERSION=%2
set PLATFORM=%3
set ZTS=%4

echo Building %PROJECT_NAME% (PHP %PHP_VERSION%; PLATFORM %PLATFORM%; %ZTS%)

if not exist %BUILD_DIR% git clone https://github.com/microsoft/php-sdk-binary-tools.git %BUILD_DIR%
if not "%errorlevel%" == "0" exit /b %errorlevel%

cd %BUILD_DIR%
phpsdk-%VC_VERSION%-%PLATFORM%.bat -t %ROOT_DIR%\platform.bat --task-args "%PHP_VERSION% %ZTS%" && echo Done building %PROJECT_NAME% && cd %ROOT_DIR%
if not "%errorlevel%" == "0" exit /b %errorlevel%

:usage
echo usage: build.bat ^<PHP_VERSION^> ^<VC_VERSION^> ^<PLATFORM^> [^<ZTS^>]
echo example: build.bat 7.4.1 vc15 x86
echo example: build.bat 8.0.0 vs16 x64 ZTS
exit 1
