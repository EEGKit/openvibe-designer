REM @ECHO OFF
SETLOCAL EnableDelayedExpansion
SETLOCAL EnableExtensions

REM Get the directory location of this script, assume it contains the OpenViBE dist tree. These variables will be used by OpenViBE executables.
SET "OV_PATH_ROOT=%~dp0"
SET "OV_PATH_BIN=%OV_PATH_ROOT%\bin"
SET "OV_PATH_LIB=%OV_PATH_ROOT%\bin"
SET "OV_PATH_DATA=%OV_PATH_ROOT%\share\openvibe"
SET "MENSIA_PATH_DATA=%OV_PATH_ROOT%\share\mensia"

SET OV_PAUSE=PAUSE
SET OV_RUN_IN_BG=

IF /i "%1"=="--no-pause" (
	SET OV_PAUSE=
	SHIFT
)
IF /i "%1"=="--run-bg" (
	REM Run in background, disable pause. The first start arg is the app title.
	SET OV_RUN_IN_BG=START "neurort-scenario-converter-real.cmd"
	SET OV_PAUSE=
	SHIFT
)

SET "OV_ENVIRONMENT_FILE=%OV_PATH_ROOT%\bin\openvibe-set-env.cmd"
IF NOT EXIST "%OV_ENVIRONMENT_FILE%" (
	ECHO Error: "%OV_ENVIRONMENT_FILE%" was not found
	GOTO EndOfScript
)
CALL "%OV_ENVIRONMENT_FILE%"

SET LUA_EXE_PATH=
FOR %%F IN (lua.exe) DO (
	SET LUA_EXE_PATH_FULL=%%~$PATH:F
	SET LUA_EXE_PATH=!LUA_EXE_PATH_FULL:lua.exe=!
)

SET PATH=%OV_PATH_LIB%;%PATH%
SET LUA_CPATH=%LUA_CPATH%;%LUA_EXE_PATH%\clibs\?.dll;%OV_PATH_ROOT%\bin\?.dll
SET LUA_PATH=%LUA_PATH%;%LUA_EXE_PATH%\lua\?.lua;%OV_PATH_ROOT%\bin\?.lua

SET TO_CONVERT=%1
setlocal enabledelayedexpansion
set CONVERT_SRC=%TO_CONVERT:\=/%

set CONVERT_DEST=%CONVERT_SRC%
set CONVERT_DEST=%CONVERT_DEST:.mxs=.mbs%
set CONVERT_DEST=%CONVERT_DEST:.mxb=.mbb%
echo %CONVERT_DEST%

lua %OV_PATH_BIN%\mensia-scenario-converter.lua %CONVERT_SRC% %CONVERT_DEST%

:EndOfScript

%OV_PAUSE%
