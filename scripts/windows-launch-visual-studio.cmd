@echo off
setlocal EnableDelayedExpansion
setlocal enableextensions 

set script_dir=%CD%

set PathSDK="%script_dir%\..\dependencies\certivibe"
set VerboseOuptut=OFF

set BuildType=Release

goto parameter_parse

:print_help
	echo Usage: windows-launch-visual-studio.cmd --sdk ^<path to openvibe SDK^>
	exit /b

:parameter_parse
for %%A in (%*) DO (
	if /i "%%A"=="--sdk" (
		set next=SDK
	) else if "!next!"=="SDK" (
		set PathSDK=%%A
		set next=
	) else if /i "%%A"=="--debug" (
		set BuildType=Debug
		set next=
	)
)

call "windows-initialize-environment.cmd" --sdk "%PathSDK%"

SET "OV_PATH_ROOT=%CD%\..\..\certivibe-build\dist-studio"
SET "OV_PATH_BIN=%OV_PATH_ROOT%\bin"
SET "OV_PATH_DATA=%OV_PATH_ROOT%\share\openvibe"
SET "OV_PATH_LIB=%OV_PATH_ROOT%\bin"
SET "PATH=%OV_PATH_ROOT%\bin;%PATH%"

REM for visual studio express...
if not defined USE_EXPRESS (
	SET USE_EXPRESS=0
)

set SolutionPath=%CD%\..\..\studio-build\studio-vs-project-%BuildType%\Designer.sln

if %USE_EXPRESS% == 1 (
	echo Use %VSCMake% Express Edition
	
	if "%VSCMake%"=="Visual Studio 12" (
		start /b "%VSINSTALLDIR%\Common7\IDE\WDExpress.exe" %SolutionPath%
	) else (
		"%VSINSTALLDIR%\Common7\IDE\VCExpress.exe" %SolutionPath%
	)
) else (
	echo Use %VSCMake% Community Edition
	"%VSINSTALLDIR%\Common7\IDE\devenv.exe" %SolutionPath%
)
