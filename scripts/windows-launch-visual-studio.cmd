@echo off
setlocal EnableDelayedExpansion
setlocal enableextensions 

call "windows-initialize-environment.cmd"

SET "OV_PATH_ROOT=%CD%\..\..\certivibe-build\dist"
SET "OV_PATH_BIN=%OV_PATH_ROOT%\bin"
SET "OV_PATH_DATA=%OV_PATH_ROOT%\share\openvibe"
SET "OV_PATH_LIB=%OV_PATH_ROOT%\bin"
SET "PATH=%OV_PATH_ROOT%\bin;%PATH%"

REM for visual studio express...
SET USE_EXPRESS=1

if %USE_EXPRESS% == 1 (
	echo Use Visual Studio Express Edition
	
	if "%VSCMake%"=="Visual Studio 12" (
		start /b "%VSINSTALLDIR%\Common7\IDE\WDExpress.exe" ..\..\certivibe-build\vs-project\OpenViBE.sln
	) else (
		"%VSINSTALLDIR%\Common7\IDE\VCExpress.exe" ..\..\certivibe-build\vs-project\OpenViBE.sln
	)
) else (
	if "%VSCMake%"=="Visual Studio 12" (
		echo Use Visual Studio Community Edition
		"%VSINSTALLDIR%\Common7\IDE\devenv.exe" ..\..\certivibe-build\vs-projectje\OpenViBE.sln
	)
)

