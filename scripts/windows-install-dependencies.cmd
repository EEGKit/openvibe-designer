REM Install Designer dependencies
REM This script can be copied, renamed as `windows-install-dependencies-custom.cmd`
REM and updated with the right credentials in PROXYPASS, such as username:password
REM 
@echo off

echo Install all dependencies

REM In order to download dependencies, PROXYPASS must be changed with appropriate credentials
REM set PROXYPASS=XXX:XXX
set URL=https://extranet.mensiatech.com/dependencies

if not exist "..\dependencies\arch\build\windows" ( mkdir "..\dependencies\arch\build\windows" )

REM Install build tools specifically needed by Designer project
REM powershell.exe -NoProfile -ExecutionPolicy Bypass -file "windows-get-dependencies.ps1" -manifest_file .\windows-build-tools.txt

REM Install OV SDK and its dependencies
powershell.exe -NoProfile -ExecutionPolicy Bypass -file "windows-get-dependencies.ps1" -manifest_file .\windows-dependencies-sdk.txt
REM Install Designer dependencies
powershell.exe -NoProfile -ExecutionPolicy Bypass -file "windows-get-dependencies.ps1" -manifest_file .\windows-dependencies.txt

pause
