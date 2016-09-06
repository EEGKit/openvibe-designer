@echo off
setlocal EnableDelayedExpansion
setlocal enableextensions 

set BuildType=Release
set PauseCommand=pause

setlocal

call "windows-initialize-environment.cmd"

set script_dir=%CD%
set build_dir=%script_dir%\..\..\certivibe-build\vs-project
set install_dir=%script_dir%\..\..\certivibe-build\dist

mkdir %build_dir% 2>NUL
pushd %build_dir%

cmake %script_dir%\.. -G"%VSCMake%" -DCMAKE_BUILD_TYPE=%BuildType% -DCMAKE_INSTALL_PREFIX=%install_dir%

if not "!ERRORLEVEL!" == "0" goto terminate_error

goto terminate_success

:terminate_error

echo An error occured during building process !

%PauseCommand%

goto terminate

:terminate_success

%PauseCommand%

goto terminate

:terminate

popd

endlocal
