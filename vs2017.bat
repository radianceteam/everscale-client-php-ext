@echo off

echo Installing Visual Studio 2017 Build Tools
choco install --yes --force visualstudio2017buildtools visualstudio2017-workload-vctools
echo DONE Installing Visual Studio 2017 Build Tools

echo Setting up env variables
dir /s/b "C:\Program Files (x86)\Microsoft Visual Studio\2017\BuildTools"
if "%3" == "x64" call "C:\Program Files (x86)\Microsoft Visual Studio\2017\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
if "%3" == "x86" call "C:\Program Files (x86)\Microsoft Visual Studio\2017\BuildTools\VC\Auxiliary\Build\vcvars32.bat"
if not "%errorlevel%" == "0" exit /b %errorlevel%
echo DONE Setting up env variables

echo Building PHP Extension
call build.bat %*
if not "%errorlevel%" == "0" exit /b %errorlevel%
echo DONE Building PHP Extension

exit 0