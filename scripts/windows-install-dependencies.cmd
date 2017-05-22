REM Install studio dependencies
REM 
@echo off

echo Install all dependencies

powershell.exe -NoProfile -ExecutionPolicy Bypass -file "windows-get-dependencies.ps1" -manifest_file .\windows-dependencies.txt
powershell.exe -NoProfile -ExecutionPolicy Bypass -file "windows-get-dependencies.ps1" -manifest_file .\windows-dependencies-sdk.txt

pause
