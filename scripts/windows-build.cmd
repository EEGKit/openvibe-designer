@echo off
setlocal EnableDelayedExpansion
setlocal enableextensions

set script_dir=%CD%

set BuildType=Release
set PauseCommand=pause
set RefreshCMake=F
set PathSDK=%script_dir%\..\dependencies\openvibe-sdk-release
set PathDep=%script_dir%\..\dependencies
set UserDataSubdir=OpenVIBE
set VerboseOuptut=OFF

goto parameter_parse

:print_help
	echo Usage: windows-build.cmd --sdk <path to openvibe SDK> --dep <path to openvibe dependencies> [--userdata-subdir <name of the userdata sub directory>] [-h ^| --help] [--no-pause] [-d^|--debug] [-r^|--release] [-f^|--force] [-v^|--verbose]
	echo -- Build Type option can be : --release (-r) or --debug (-d). Default is Release.
	echo -- --force option will force the cmake re-run
	exit /b

:parameter_parse
for %%A in (%*) DO (
	if /i "%%A"=="-h" (
		goto print_help
	) else if /i "%%A"=="--help" (
		goto print_help
	) else if /i "%%A"=="--no-pause" (
		set PauseCommand=echo ""
	) else if /i "%%A"=="-d" (
		set BuildType=Debug
	) else if /i "%%A"=="--debug" (
		set BuildType=Debug
	) else if /i "%%A"=="-r" (
		set BuildType=Release
	) else if /i "%%A"=="--release" (
		set BuildType=Release
	) else if /i "%%A"=="-f" (
		set RefreshCMake=T
	) else if /i "%%A"=="--force" (
		set RefreshCMake=T
	) else if /i "%%A"=="-v" (
		set VerboseOutput=ON
	) else if /i "%%A"=="--verbose" (
		set VerboseOutput=ON
	) else if /i "%%A"=="--sdk" (
		set next=SDK
	) else if "!next!"=="SDK" (
		set PathSDK=%%A
		set next=
	) else if /i "%%A"=="--dep" (
		set next=DEP
	) else if "!next!"=="DEP" (
		set PathDep=%%A
		set next=
	) else if /i "%%A"=="--userdata-subdir" (
    set next=USERDATA_SUBDIR
  ) else if "!next!"=="USERDATA_SUBDIR" (
    set UserDataSubdir=%%A
    set next=
	)
)

setlocal

call "windows-initialize-environment.cmd" --dep %PathDep%

set build_dir=%script_dir%\..\..\certivibe-build\build-studio-%BuildType%
if %PathSDK%=="" (
  set sdk_dir=%script_dir%\..\dependencies\openvibe-sdk-release
) else (
	set sdk_dir=%PathSDK%
)

if %PathDep%=="" (
	set dep_dir=%script_dir%\..\dependencies\
) else (
	set dep_dir=%PathDep%
)

if "%BuildType%"=="Debug" (
	set install_dir=%script_dir%\..\..\certivibe-build\dist-studio-debug
) else (
	set install_dir=%script_dir%\..\..\certivibe-build\dist-studio
)

mkdir %build_dir% 2>NUL
pushd %build_dir%

echo Build type is set to: %BuildType%. SDK is located at %sdk_dir%

if not exist "%build_dir%\CMakeCache.txt" set RefreshCMake=T
if "%RefreshCMake%"=="T" (
	cmake -DFlag_VerboseOutput=%VerboseOutput% %script_dir%\.. -G"Ninja" -DCMAKE_BUILD_TYPE=!BuildType! -DCMAKE_INSTALL_PREFIX=!install_dir! -DOPENVIBE_SDK_PATH=!sdk_dir! -DCV_DEPENDENCIES_PATH=!dep_dir! -DOV_CONFIG_SUBDIR=%UserDataSubdir%

)

if not "!ERRORLEVEL!" == "0" goto terminate_error

ninja install

if not "!ERRORLEVEL!" == "0" goto terminate_error

goto terminate_success

:terminate_error

echo An error occured during building process !

%PauseCommand%

goto terminate

:terminate_success

%PauseCommand%

goto terminate

:terminate

popd

endlocal
