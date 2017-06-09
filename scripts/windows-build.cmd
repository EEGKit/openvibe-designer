@echo off
setlocal EnableDelayedExpansion
setlocal enableextensions

pushd %~dp0\..
set "root_dir=%CD%"
popd

set BuildType=Release
set PauseCommand=pause
set RefreshCMake=F
set PathSDK=
set PathDep=%root_dir%\dependencies
set OEMDistribution=openvibe
set VerboseOuptut=OFF
set DisplayErrorLocation=ON

goto parameter_parse

:print_help
	echo Usage: %0 [options]
    echo.
	echo [-h ^| --help]   display usage
	echo [--no-pause]     will not pause during script execution
	echo [-d^|--debug]    build in debug mode
	echo [-r^|--release]  build in release mode
	echo [--make-package] build zip packages instead of installing files
	echo [--hide-error-location] do not display complete error locations
	echo [-f^|--force]    force option will force the cmake re-run
	echo [-v^|--verbose]
	echo [--sdk <path to openvibe SDK>]
	echo [--dep <path to openvibe dependencies>]
	echo [--build-dir <dirname>] build directory
	echo [--install-dir <dirname>] binaries deployment directory
	echo [--oem-distribution <name of the distribution>]
	echo -- Build Type option can be : --release (-r) or --debug (-d). Default is Release.
	exit /b

:parameter_parse
if /i "%1"=="-h" (
	goto print_help
) else if /i "%1"=="--help" (
	goto print_help
) else if /i "%1"=="--no-pause" (
	set PauseCommand=echo ""
	SHIFT
	Goto parameter_parse
) else if /i "%1"=="-d" (
	set BuildType=Debug
	SHIFT
	Goto parameter_parse
) else if /i "%1"=="--debug" (
	set BuildType=Debug
	SHIFT
	Goto parameter_parse
) else if /i "%1"=="-r" (
	set BuildType=Release
	SHIFT
	Goto parameter_parse
) else if /i "%1"=="--release" (
	set BuildType=Release
	SHIFT
	Goto parameter_parse
) else if /i "%1"=="--make-package" (
	set PackageOption=TRUE
	set DisplayErrorLocation=OFF
	SHIFT
	Goto parameter_parse
) else if /i "%1" == "--hide-error-location" (
	set DisplayErrorLocation=OFF
	SHIFT
	Goto parameter_parse
) else if /i "%1"=="-f" (
	set RefreshCMake=T
	SHIFT
	Goto parameter_parse
) else if /i "%1"=="--force" (
	set RefreshCMake=T
	SHIFT
	Goto parameter_parse
) else if /i "%1"=="-v" (
	set VerboseOutput=ON
	SHIFT
	Goto parameter_parse
) else if /i "%%1"=="--verbose" (
	set VerboseOutput=ON
	SHIFT
	Goto parameter_parse
) else if /i "%1"=="--sdk" (
	set PathSDK=%2
	SHIFT
	SHIFT
	Goto parameter_parse
) else if /i "%1"=="--dependencies-dir" (
	set dependencies_dir=-DLIST_DEPENDENCIES_PATH=%2
	set otherdep=%2
	SHIFT
	SHIFT
	Goto parameter_parse
) else if /i "%1"=="--oem-distribution" (
	set OEMDistribution=%2
	SHIFT
	SHIFT
	Goto parameter_parse
) else if /i "%1"=="--build-dir" (
	set build_dir=%2
	SHIFT
	SHIFT
	Goto parameter_parse
) else if /i "%1"=="--install-dir" (
	set install_dir=%2
	SHIFT
	SHIFT
	Goto parameter_parse
) else if not "%1" == "" (
	echo unrecognized option [%1]
	Goto terminate_error
)

setlocal

call "windows-initialize-environment.cmd" %PathDep% %otherdep%

if not defined build_dir (
	set build_dir=%root_dir%\..\certivibe-build\build-studio-%BuildType%
)
if not defined install_dir (
	if "%BuildType%"=="Debug" (
		set install_dir=%root_dir%\..\certivibe-build\dist-studio-debug
	) else (
		set install_dir=%root_dir%\..\certivibe-build\dist-studio
	)
)

set sdk_dir=%PathSDK%
set dep_dir=%PathDep%

mkdir %build_dir% 2>NUL
pushd %build_dir%

echo Build type is set to: %BuildType%. SDK is located at %sdk_dir%

set CallCmake=false
if not exist "%build_dir%\CMakeCache.txt" set CallCmake="true"
if %RefreshCMake%==T set CallCmake="true"
if %CallCmake%=="true" (
	cmake %root_dir%\ -G"Ninja" ^
		-DCMAKE_BUILD_TYPE=%BuildType% ^
		-DCMAKE_INSTALL_PREFIX=%install_dir% ^
		-DOPENVIBE_SDK_PATH=!sdk_dir! ^
		-DOV_DISPLAY_ERROR_LOCATION=%DisplayErrorLocation% ^
		-DCV_DEPENDENCIES_PATH=!dep_dir!^
		-DOEM_DISTRIBUTION=%OEMDistribution% ^
		-DOV_PACKAGE=%PackageOption% ^
		-DFlag_VerboseOutput=%VerboseOutput% ^
		%dependencies_dir%
)
if not "!ERRORLEVEL!" == "0" goto terminate_error

ninja install

if not "!ERRORLEVEL!" == "0" goto terminate_error

if "%PackageOption%"=="TRUE" (
	cmake --build . --target package
)


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

