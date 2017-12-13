@echo off

set USE_EXPRESS=0
REM set SKIP_VS2015=0
call "windows-build.cmd" --vsproject %*
