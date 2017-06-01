@echo off
setlocal EnableDelayedExpansion
setlocal enableextensions

pushd %~dp0\..
set "root_dir=%CD%"
popd

REM To compile with the Visual Studio toolset v120
set SKIP_VS2017=1
set SKIP_VS2015=1

set BuildType=Release
set PauseCommand=pause
set RefreshCMake=F
set PathSDK=
set PathDep=%root_dir%\dependencies
set OEMDistribution=openvibe
set VerboseOuptut=OFF

goto parameter_parse

:print_help
	echo Usage: %0 [options]
    echo.
	echo [-h ^| --help]   display usage
	echo [--no-pause]     will not pause during script execution
	echo [-d^|--debug]    build in debug mode
	echo [-r^|--release]  build in release mode
	echo [--make-package] build zip packages instead of installing files
	echo [-f^|--force]    force option will force the cmake re-run
	echo [-v^|--verbose]
	echo [--sdk <path to openvibe SDK>]
	echo [--dep <path to openvibe dependencies>]
	echo [--oem-distribution <name of the distribution>]
	echo -- Build Type option can be : --release (-r) or --debug (-d). Default is Release.
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
	) else if /i "%%A"=="--make-package" (
        set PackageOption=TRUE
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
	) else if /i "%%A"=="--oem-distribution" (
        set next=OEM_DISTRIBUTION
    ) else if "!next!"=="OEM_DISTRIBUTION" (
        set OEMDistribution=%%A
        set next=
	)
)
setlocal

call "windows-initialize-environment.cmd" --dep %PathDep%

set build_dir=%root_dir%\..\certivibe-build\build-studio-%BuildType%

if "%BuildType%"=="Debug" ( 
	set install_dir=%root_dir%\..\certivibe-build\dist-studio-debug 
) else ( 
	set install_dir=%root_dir%\..\certivibe-build\dist-studio 
) 

set sdk_dir=%PathSDK%
set dep_dir=%PathDep%

mkdir %build_dir% 2>NUL
pushd %build_dir%

echo Build type is set to: %BuildType%. SDK is located at %sdk_dir%

set CallCmake=false
if not exist "%build_dir%\CMakeCache.txt" set CallCmake="true"
if %RefreshCMake%=="true" set CallCmake="true"
if %CallCmake%=="true" (
	cmake %root_dir%\ -G"Ninja" ^
		-DCMAKE_BUILD_TYPE=%BuildType% ^
		-DCMAKE_INSTALL_PREFIX=%install_dir% ^
        -DOPENVIBE_SDK_PATH=!sdk_dir! ^
        -DCV_DEPENDENCIES_PATH=!dep_dir!^
        -DOEM_DISTRIBUTION=%OEMDistribution% ^
		-DOV_PACKAGE=%PackageOption% ^
        -DFlag_VerboseOutput=%VerboseOutput%
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
