set PATH=C:\Program Files (x86)\Microsoft SDKs\Windows\v7.0A\bin;%PATH%
set "SCRIPT_PATH=%~dp0"
set "PATH_DEPENDENCIES=%SCRIPT_PATH%\..\dependencies"

set PATH=%PATH_DEPENDENCIES%\gtk\bin;%PATH%

set VSTOOLS=
set VSCMake=

if exist "%VS120COMNTOOLS%vsvars32.bat" (
	echo Found VS120 tools at "%VS120COMNTOOLS%" ...
	call "%VS120COMNTOOLS%vsvars32.bat"
	set VSCMake=Visual Studio 12
)

:parameter_parse
for %%A in (%*) DO (
	if /i "%%A"=="--sdk" (
		set next=SDK
	) else if "!next!"=="SDK" (
		set PathSDK=%%A
		set next=
	)
)

set script_dir=%CD%
set build_dir=%script_dir%\..\build\build-%BuildType%
if %PathSDK%=="" (
	set sdk_dir=%script_dir%\..\dependencies\certivibe
) else (
	set sdk_dir=%PathSDK%
)

call "%sdk_dir%/bin/openvibe-set-env.cmd"

