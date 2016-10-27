@echo off
setlocal EnableDelayedExpansion
setlocal enableextensions 

set PathSDK=""
set VerboseOuptut=OFF

goto parameter_parse

:print_help
	echo Usage: winndows-generate-vs-project.cmd --sdk <path to openvibe SDK>
	exit /b

:parameter_parse
for %%A in (%*) DO (
	if /i "%%A"=="--sdk" (
		set next=SDK
	) else if "!next!"=="SDK" (
		set PathSDK=%%A
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
SET USE_EXPRESS=1

if %USE_EXPRESS% == 1 (
	echo Use Visual Studio Express Edition
	
	if "%VSCMake%"=="Visual Studio 12" (
		start /b "%VSINSTALLDIR%\Common7\IDE\WDExpress.exe" ..\..\certivibe-build\studio-vs-project\OpenViBE.sln
	) else (
		"%VSINSTALLDIR%\Common7\IDE\VCExpress.exe" ..\..\certivibe-build\studio-vs-project\OpenViBE.sln
	)
) else (
	if "%VSCMake%"=="Visual Studio 12" (
		echo Use Visual Studio Community Edition
		"%VSINSTALLDIR%\Common7\IDE\devenv.exe" ..\..\certivibe-build\studio-vs-project\OpenViBE.sln
	)
)

