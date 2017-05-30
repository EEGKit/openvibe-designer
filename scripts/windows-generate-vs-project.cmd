@echo off
setlocal EnableDelayedExpansion
setlocal enableextensions 

set script_dir=%CD%

set BuildType=Release
set PauseCommand=pause
set RefreshCMake=F
set VerboseOuptut=OFF

goto parameter_parse

:print_help
	echo Usage: windows-generate-vs-project.cmd --sdk ^<path to openvibe SDK^> --dep ^<path to openvibe dependencies^> [--debug]
	goto :terminate

:parameter_parse

for %%A in (%*) DO (
	if /i "%%A"=="--sdk" (
		set next=SDK
	) else if "!next!"=="SDK" (
		set PathSDK=%%A
		set next=
	) else if /i "%%A"=="--dep" (
		set next=DEP
	) else if "!next!"=="DEP" (
		set PathDep=%%A
		set next=
	) else if /i "%%A"=="--debug" (
		set BuildType=Debug
		set next=
	) else if /i "%%A"=="--help" (
		goto :print_help
	)
)

setlocal

if not defined PathSDK (
	set PathSDK="%script_dir%/../dependencies/openvibe-sdk-release"
	echo No SDK's path provided. Set to the default one: !PathSDK!
)

if not defined PathDep (
	set PathDep="%script_dir%/../dependencies/"
	echo No dependencies's path provided. Set to the default one: !PathDep!
)

REM To compile with the Visual Studio toolset v120
set SKIP_VS2017=1
set SKIP_VS2015=1

call "windows-initialize-environment.cmd" --dep "!PathDep!"

set build_dir=%script_dir%\..\..\certivibe-build\studio-vs-project
set install_dir=%script_dir%\..\..\certivibe-build\dist-studio

echo install_dir: %install_dir%

mkdir %build_dir% 2>NUL
pushd %build_dir%

echo SDK's path: !PathSDK!
echo Dependencies's path: !PathDep!

cmake %script_dir%\.. ^
	-G"%VSCMake%" ^
	-T "v120" ^
	-DCMAKE_BUILD_TYPE=%BuildType% ^
	-DCMAKE_INSTALL_PREFIX=%install_dir% ^
	-DOPENVIBE_SDK_PATH=!PathSDK! ^
	-DCV_DEPENDENCIES_PATH=!PathDep!

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
