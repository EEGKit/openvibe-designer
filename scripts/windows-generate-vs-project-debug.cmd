@echo off
setlocal EnableDelayedExpansion
setlocal enableextensions 

set script_dir=%CD%

set BuildType=Debug
set PauseCommand=pause
set RefreshCMake=F
set PathSDK="%script_dir%\..\dependencies\certivibe-debug"
set PathDep="%script_dir%\..\dependencies\certivibe-dependencies"
set VerboseOuptut=OFF

goto parameter_parse

:print_help
	echo Usage: winndows-generate-vs-project.cmd --sdk <path to openvibe SDK> --dep <path to openvibe dependencies>
	exit /b

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
	)
)
setlocal

call "windows-initialize-environment.cmd" --sdk "%PathSDK%"

set build_dir=%script_dir%\..\..\certivibe-build\studio-vs-project-debug
set install_dir=%script_dir%\..\..\certivibe-build\dist-studio-debug

mkdir %build_dir% 2>NUL
pushd %build_dir%

cmake %script_dir%\.. -G"%VSCMake%" -DCMAKE_BUILD_TYPE=%BuildType% -DCMAKE_INSTALL_PREFIX=%install_dir% -DOPENVIBE_SDK_PATH=!PathSDK! -DCV_DEPENDENCIES_PATH=!PathDep!

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
