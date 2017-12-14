@echo off
setlocal EnableDelayedExpansion
setlocal enableextensions 

set script_dir=%CD%

set PathSDK="%script_dir%\..\dependencies\openvibe-sdk-release"
set VerboseOuptut=OFF
set PlatformTarget=x86
set PATH_DEPENDENCIES=

set BuildType=Release

goto parameter_parse

:print_help
	echo Usage: windows-launch-visual-studio.cmd [--debug] [--sdk ^<path to openvibe SDK^>] [--dependencies-dir ^<path to dependencies^>]
	exit /b

:parameter_parse
if /i "%1" == "--dependencies-dir" (
 	set "PATH_DEPENDENCIES=%2"
 	SHIFT
 	SHIFT
 	Goto parameter_parse
)  else if /i "%1"=="--debug" (
	set BuildType=Debug
	set PathSDK="%script_dir%\..\dependencies\openvibe-sdk-debug"
	SHIFT
) else if /i "%1"=="--sdk" (
	set PathSDK=%2
	SHIFT
	SHIFT
 	Goto parameter_parse
) else if /i "%1" == "--platform-target" (
	if "%2"=="x64" (
		set PlatformTarget=%2
		SHIFT
		SHIFT

		Goto parameter_parse
	)
) 


call "windows-initialize-environment.cmd" --platform-target %PlatformTarget%

SET "OV_PATH_ROOT=%CD%\..\..\openvibe-designer-build\dist\%BuildType%-%PlatformTarget%"
SET "PATH=%OV_PATH_ROOT%\bin;%PATH%"

REM for visual studio express...
if not defined USE_EXPRESS (
	SET USE_EXPRESS=1
)

set SolutionPath=%CD%\..\..\openvibe-designer-build\vs-project-%PlatformTarget%\Designer.sln

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
