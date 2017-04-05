set PATH=C:\Program Files (x86)\Microsoft SDKs\Windows\v7.0A\bin;%PATH%
set "SCRIPT_PATH=%~dp0"
set "PATH_DEPENDENCIES=%SCRIPT_PATH%\..\dependencies"
set "PathDep=%SCRIPT_PATH%\..\dependencies"

set PATH=%PATH_DEPENDENCIES%\gtk\bin;%PATH%

rem Inject the necessary dependencies from the Certivibe project

:parameter_parse
for %%A in (%*) DO (
	if /i "%%A"=="--dep" (
		set next=DEP
	) else if "!next!"=="DEP" (
		set PathDep=%%A
		set next=
	)
)

set PATH=%PathDep%\cmake\bin;%PATH%
set PATH=%PathDep%\ninja;%PATH%

set VSTOOLS=
set VSCMake=

if exist "%VS120COMNTOOLS%vsvars32.bat" (
	echo Found VS120 tools at "%VS120COMNTOOLS%" ...
	call "%VS120COMNTOOLS%vsvars32.bat"
	set VSCMake=Visual Studio 12
)


