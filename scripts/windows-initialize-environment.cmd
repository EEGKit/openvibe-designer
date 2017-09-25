@echo off
REM setlocal EnableDelayedExpansion
REM setlocal enableextensions 

set PATH=C:\Program Files (x86)\Microsoft SDKs\Windows\v7.0A\bin;%PATH%
set "SCRIPT_PATH=%~dp0"

:parameter_parse
 
 if /i "%1" == "--dependencies-dir" (
 	set "PATH_DEPENDENCIES=%2"
 	SHIFT
 	SHIFT
 	Goto parameter_parse
 ) else if /i "%1" == "--platform-target" (
 	if "%2"=="x64" (
 		set PLATFORM=x64
 		set VSPLATFORMGENERATOR=Win64
 	) else if "%2"=="x86" (
		set PLATFORM=x86
		set VSPLATFORMGENERATOR=
 	) else (
		echo Unknown platform %2 target
		Goto terminate
 	)
	SHIFT
	SHIFT

	Goto parameter_parse
) else if not "%1" == "" (
	echo unrecognized option [%1]
	Goto terminate_error
)

if not defined PATH_DEPENDENCIES (
	if %PLATFORM%==x64 (
		SET "PATH_DEPENDENCIES=%SCRIPT_PATH%../dependencies_x64"
	) else (
		SET "PATH_DEPENDENCIES=%SCRIPT_PATH%../dependencies"
	)

	set "PATH=!PATH_DEPENDENCIES!/cmake/bin;!PATH!"
	set "PATH=!PATH_DEPENDENCIES!/ninja;!PATH!"
	set "PATH=!PATH_DEPENDENCIES!/gtk/bin;!PATH!"
) else (
	for %%A in (%PATH_DEPENDENCIES%) DO (
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

set VCVARSALLPATH=../../VC/vcvarsall.bat

if %SKIP_VS2017% == 1 (
	echo Visual Studio 2017 detection skipped as requested
) else (
	if exist "%VS150COMNTOOLS%%VCVARSALLPATH%" (
		echo Found VS150 tools at "%VS150COMNTOOLS%%VCVARSALLPATH%" ...
		if %PLATFORM% == x64 (
			if exist "%VS150COMNTOOLS%/../../bin/x64" (
				call "%VS150COMNTOOLS%%VCVARSALLPATH%" %PLATFORM%
			) else (
				call "%VS150COMNTOOLS%%VCVARSALLPATH%" x86_amd64
			)
		) else (
			call "%VS150COMNTOOLS%%VCVARSALLPATH%" %PLATFORM%
 		)
		set VSCMake=Visual Studio 15 2017
		if %PLATFORM% == x64 (
			set VSCMake=!VSCMake! %VSPLATFORMGENERATOR%
		)
		goto terminate
	)
)

if %SKIP_VS2015% == 1 (
	echo Visual Studio 2015 detection skipped as requested
) else (
	if exist "%VS140COMNTOOLS%%VCVARSALLPATH%" (
		echo Found VS140 tools at "%VS140COMNTOOLS%%VCVARSALLPATH%" ...
		if %PLATFORM% == x64 (
			if exist "%VS140COMNTOOLS%/../../bin/x64" (
				call "%VS140COMNTOOLS%%VCVARSALLPATH%" %PLATFORM%
			) else (
				call "%VS140COMNTOOLS%%VCVARSALLPATH%" x86_amd64
			)
		) else (
			call "%VS140COMNTOOLS%%VCVARSALLPATH%" %PLATFORM%
 		)
		set VSCMake=Visual Studio 14 2015
		if %PLATFORM% == x64 (
			set VSCMake=!VSCMake! %VSPLATFORMGENERATOR%
		)
		goto terminate
	)
)

if %SKIP_VS2013% == 1 (
	echo Visual Studio 2013 detection skipped as requested
) else (
	if exist "%VS120COMNTOOLS%%VCVARSALLPATH%" (
		echo Found VS120 tools at "%VS120COMNTOOLS%%VCVARSALLPATH%" ...
		if %PLATFORM% == x64 (
			if exist "%VS120COMNTOOLS%../../VC/bin/x64" (
				call "%VS120COMNTOOLS%%VCVARSALLPATH%" %PLATFORM%
			) else (
				call "%VS120COMNTOOLS%%VCVARSALLPATH%" x86_amd64
			)
		) else (
			call "%VS120COMNTOOLS%%VCVARSALLPATH%" %PLATFORM%
 		)
		set VSCMake=Visual Studio 12 2013
		if %PLATFORM% == x64 (
			set VSCMake=!VSCMake! %VSPLATFORMGENERATOR%
		)
		goto terminate
	)
)

goto terminate_success

:terminate_error

echo An error occured during environment initializing !

goto terminate

:terminate_success

goto terminate

:terminate

popd

endlocal
