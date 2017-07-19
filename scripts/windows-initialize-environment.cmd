@echo off
REM setlocal EnableDelayedExpansion
REM setlocal enableextensions 

set PATH=C:\Program Files (x86)\Microsoft SDKs\Windows\v7.0A\bin;%PATH%
set "SCRIPT_PATH=%~dp0"

if "%1"=="" (
	set args=%SCRIPT_PATH%\..\dependencies
) else (
	set args=%*
)

rem Inject the necessary dependencies from the OpenViBE SDK project
REM echo %PATH%

for %%A in (%args%) DO (
	if exist "%%A\cmake\bin\" (
		set "PATH=%%A\cmake\bin;!PATH!"
	)
	if exist "%%A\gtk\bin\" (
		set "PATH=%%A\gtk\bin;!PATH!"
	)
	if exist "%%A\ninja\" (
		set "PATH=%%A\ninja;!PATH!"
	)
)

REM # Set to 1 to skip new compilers.
if not defined SKIP_VS2017 (
	SET SKIP_VS2017=1
)
if not defined SKIP_VS2015 (
	SET SKIP_VS2015=1
)
if not defined SKIP_VS2013 (
	SET SKIP_VS2013=0
)

set VSTOOLS=
set VSCMake=

if %SKIP_VS2017% == 1 (
	echo Visual Studio 2017 detection skipped as requested
) else (
	if exist "%VS150COMNTOOLS%vsvars32.bat" (
		echo Found VS150 tools at "%VS150COMNTOOLS%" ...
		CALL "%VS150COMNTOOLS%vsvars32.bat"
		SET VSCMake=Visual Studio 15
		goto terminate
	)
)

if %SKIP_VS2015% == 1 (
	echo Visual Studio 2015 detection skipped as requested
) else (
	if exist "%VS140COMNTOOLS%vsvars32.bat" (
		echo Found VS140 tools at "%VS140COMNTOOLS%" ...
		CALL "%VS140COMNTOOLS%vsvars32.bat"
		SET VSCMake=Visual Studio 14
		goto terminate
	)
)

if %SKIP_VS2013% == 1 (
	echo Visual Studio 2013 detection skipped as requested
) else (
	if exist "%VS120COMNTOOLS%vsvars32.bat" (
		echo Found VS120 tools at "%VS120COMNTOOLS%" ...
		CALL "%VS120COMNTOOLS%vsvars32.bat"
		SET VSCMake=Visual Studio 12
		goto terminate
	)
)

:terminate

popd

endlocal
