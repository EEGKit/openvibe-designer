set PATH=C:\Program Files (x86)\Microsoft SDKs\Windows\v7.0A\bin;%PATH%
set "SCRIPT_PATH=%~dp0"
set "PATH_DEPENDENCIES=%SCRIPT_PATH%\..\dependencies"

set PATH=%PATH_DEPENDENCIES%\boost\bin;%PATH%
set PATH=%PATH_DEPENDENCIES%\cmake\bin;%PATH%
set PATH=%PATH_DEPENDENCIES%\expat\bin;%PATH%
set PATH=%PATH_DEPENDENCIES%\ninja;%PATH%
set PATH=%PATH_DEPENDENCIES%\tvicport\bin;%PATH%
set PATH=%PATH_DEPENDENCIES%\vcredist;%PATH%
set PATH=%PATH_DEPENDENCIES%\zip;%PATH%

set VSTOOLS=
set VSCMake=

if exist "%VS120COMNTOOLS%vsvars32.bat" (
	echo Found VS120 tools at "%VS120COMNTOOLS%" ...
	call "%VS120COMNTOOLS%vsvars32.bat"
	set VSCMake=Visual Studio 12
)

