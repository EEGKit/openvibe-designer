	SetCompressor /FINAL /SOLID lzma
	SetCompressorDictSize 16

	!include "MUI.nsh"
	!include "Sections.nsh"
	!include "LogicLib.nsh"


	;Name and file
	Name "OpenViBE dependencies (Mensia)"
	OutFile "win32-install_dependencies_mensia.exe"

	;Default installation folder
	InstallDir "$EXEDIR\..\dependencies"

;Interface Settings

	!define MUI_ABORTWARNING
	!define MUI_COMPONENTSPAGE_NODESC

;Pages

	!insertmacro MUI_PAGE_COMPONENTS
	!insertmacro MUI_PAGE_INSTFILES

    !insertmacro MUI_UNPAGE_CONFIRM
	!insertmacro MUI_UNPAGE_INSTFILES

;Languages

	!insertmacro MUI_LANGUAGE "English"

;Installer and uninstaller icons

	Icon "${NSISDIR}\Contrib\Graphics\Icons\box-install.ico"
	UninstallIcon "${NSISDIR}\Contrib\Graphics\Icons\box-uninstall.ico"

;VS90/VS100 suffix

	Var suffix

; manifest File for cmake checking
	!define DEPENDENCIES_MANIFEST "$INSTDIR\manifest.txt"
; set-env File for PATH modification
	!define SET_ENV_SCRIPT "$EXEDIR\win32-dependencies.cmd"

;##########################################################################################################################################################
; MACROS
;##########################################################################################################################################################
	; Add a software to the set-env script
	; param: installFolder (added to PATH, it may be something like 'folder/bin', 'folder/lib' or 'folder')
	!define AddToSetEnv `!insertmacro ADD_TO_SET_ENV`
	!macro ADD_TO_SET_ENV installFolder
		FileOpen $1 ${SET_ENV_SCRIPT} a
		FileSeek $1 0 END
		FileWrite $1 "SET PATH=$INSTDIR\${installFolder};%PATH%$\r$\n"
		FileClose $1
	!macroend

	; Add a software to the version manifest
	; param: name (str, must match the cmake manifest), version (str, will be str-compared)
	!define AddToManifest `!insertmacro ADD_TO_MANIFEST`
	!macro ADD_TO_MANIFEST name version
		FileOpen $1 "${DEPENDENCIES_MANIFEST}" a
		FileSeek $1 0 END
		FileWrite $1 "${name}=${version}$\r$\n"
		FileClose $1
	!macroend

	!define ExtractZip `!insertmacro EXTRACT_ZIP`
	!macro EXTRACT_ZIP archiveName installFolder
		ZipDLL::extractall "arch\${archiveName}" "${installFolder}"
	!macroend

	; download  a dependency on https links, such as dropbox
	; param, installFolder (will create 'dependencies/name'), archiveName, HTTPSurl (don't forget the suffix '?dl=1' for dropbox links)
	!define DownloadFromHTTPS `!insertmacro DOWNLOAD_FROM_HTTPS`
	!macro DOWNLOAD_FROM_HTTPS archiveName HTTPSurl
		IfFileExists "arch\${archiveName}" +8
			inetc::get "${HTTPSurl}" "arch\${archiveName}" /END
			Pop $R0 ; Get the return value
				StrCmp $R0 "OK" +5
					MessageBox MB_OK "Download file [${archiveName}] failed: $R0" /SD IDOK
					IfFileExists "arch\${archiveName}" 0 +2
						Delete "arch\${archiveName}" ; to remove the html artifact that may have been downloaded through dropbox
					Quit
	!macroend

	!define DownloadDocFromHTTPS `!insertmacro DOWNLOAD_DOC_FROM_HTTPS`
	!macro DOWNLOAD_DOC_FROM_HTTPS docName HTTPSurl
		IfFileExists "${docName}" +6
			inetc::get "${HTTPSurl}" "${docName}" /END
			Pop $R0 ; Get the return value
				StrCmp $R0 "OK" +3
					MessageBox MB_OK "Download file [${docName}] WebApps User Manual download failed: $R0" /SD IDOK
					Quit
	!macroend

	!define DownloadFromHTTPSAndExtract `!insertmacro DOWNLOAD_FROM_HTTPS_AND_EXTRACT`
	!macro DOWNLOAD_FROM_HTTPS_AND_EXTRACT installFolder archiveName HTTPSurl
		${DownloadFromHTTPS} ${archiveName} ${HTTPSurl}
		${ExtractZip} ${archiveName}  ${installFolder}
	!macroend


	; download  a dependency on a http link
	; param, installFolder (will create 'dependencies/name'), archiveName, dropboxURL (don't forget the suffix '?dl=1')
	!define DownloadFromHTTP `!insertmacro DOWNLOAD_FROM_HTTP`
	!macro DOWNLOAD_FROM_HTTP archiveName HTTPurl
		IfFileExists "arch\${archiveName}" +6
			NSISdl::download ${HTTPurl} "arch\${archiveName}"
			Pop $R0 ; Get the return value
				StrCmp $R0 "success" +3
					MessageBox MB_OK "Download file [${archiveName}] failed: $R0" /SD IDOK
					Quit
	!macroend

	!define DownloadFromHTTPAndExtract `!insertmacro DOWNLOAD_FROM_HTTP_AND_EXTRACT`
	!macro DOWNLOAD_FROM_HTTP_AND_EXTRACT installFolder archiveName HTTPurl
		${DownloadFromHTTP} ${archiveName} ${HTTPurl}
		${ExtractZip} ${archiveName}  ${installFolder}
	!macroend

;##########################################################################################################################################################

Section "-base"

	;Finds Microsoft Platform SDK

	ReadRegStr $r0 HKLM "SOFTWARE\Microsoft\Win32SDK\Directories" "Install Dir"
	StrCmp $r0 "" base_failed_to_find_sdk_1 base_found_sdk
base_failed_to_find_sdk_1:
	ReadRegStr $r0 HKLM "SOFTWARE\Microsoft\MicrosoftSDK\Directories" "Install Dir"
	StrCmp $r0 "" base_failed_to_find_sdk_2 base_found_sdk
base_failed_to_find_sdk_2:
	ReadRegStr $r0 HKLM "SOFTWARE\Microsoft\Microsoft SDKs\Windows" "CurrentInstallFolder"
	StrCmp $r0 "" base_failed_to_find_sdk_3 base_found_sdk
base_failed_to_find_sdk_3:
	goto base_failed_to_find_sdk

base_failed_to_find_sdk:
	MessageBox MB_OK|MB_ICONEXCLAMATION "Failed to find Microsoft Platform SDK$\nPlease update your win32-dependencies.cmd script by hand" /SD IDOK
	goto base_go_on
base_found_sdk:
	MessageBox MB_OK "Microsoft Platform SDK found at :$\n$r0" /SD IDOK
	goto base_go_on

base_go_on:

	;Create uninstaller
	WriteUninstaller "$INSTDIR\Uninstall.exe"

	;clears dependencies file
	FileOpen $0 "$EXEDIR\win32-dependencies.cmd" w
	FileWrite $0 "@echo off$\r$\n"
	FileWrite $0 "$\r$\n"
	FileWrite $0 "SET PATH=$r0\bin;%PATH%$\r$\n"
	FileClose $0

	; clean manifest
	FileOpen $1 "${DEPENDENCIES_MANIFEST}" w
	FileClose $1
SectionEnd

;##########################################################################################################################################################

SectionGroup "!Compilation platform"
Section /o "Visual C++ 2008" vs90
	StrCpy $suffix "vs90"
SectionEnd

Section "Visual C++ 2010" vs100
	StrCpy $suffix "vs100"
SectionEnd

; We need to recompile ogg, vorbix and VRPN before activating this option.
; Section "Visual C++ 2013" vs120
	; StrCpy $suffix "vs120"
; SectionEnd
SectionGroupEnd

;##########################################################################################################################################################

Section "DirectX Runtime"

	SetOutPath "$INSTDIR"
	CreateDirectory "$INSTDIR\arch"

	; even if we won't install it, we download directx anyway to be able to make the packages.
	${DownloadFromHTTPS} "directx_aug2009_redist.exe" "https://www.dropbox.com/s/bwvut8rt0f94zxo/directx_aug2009_redist.exe?dl=1"

	IfFileExists "$SYSDIR\d3dx9_42.dll" no_need_to_install_directx
	ExecWait '"arch\directx_aug2009_redist.exe" /T:"$INSTDIR\tmp" /Q'
	ExecWait '"$INSTDIR\tmp\DXSETUP.exe" /silent'
no_need_to_install_directx:

SectionEnd

;##########################################################################################################################################################

Section "CMake"

	SetOutPath "$INSTDIR"
	CreateDirectory "$INSTDIR\arch"

	${DownloadFromHTTPSAndExtract} "cmake" "cmake-2.8.12.1-win32-x86-mensia.zip" "https://www.dropbox.com/s/bxwzmrdwlgbcn58/cmake-2.8.12.1-win32-x86-mensia.zip?dl=1"

SectionEnd

;##########################################################################################################################################################

Section "eXpat"

	SetOutPath "$INSTDIR"
	CreateDirectory "$INSTDIR\arch"

	${DownloadFromHTTPSAndExtract} "expat" "expat-2.0.1-dev.zip" "https://www.dropbox.com/s/j1yy94o35ab219z/expat-2.0.1-dev.zip?dl=1"
	${DownloadFromHTTPSAndExtract} "expat" "expat-2.0.1-runtime.zip" "https://www.dropbox.com/s/vanrsqrqm3d5wfc/expat-2.0.1-runtime.zip?dl=1"

SectionEnd

;##########################################################################################################################################################

Section "BOOST"

	SetOutPath "$INSTDIR"
	CreateDirectory "$INSTDIR\arch"

	${DownloadFromHTTPSAndExtract} "boost" "boost-1.55.zip" "https://www.dropbox.com/s/2f0ppk03bo0iiev/boost-1.55.zip?dl=1"

SectionEnd

;##########################################################################################################################################################

Section "GTK+"

	SetOutPath "$INSTDIR"
	CreateDirectory "$INSTDIR\arch"

	${DownloadFromHTTPSAndExtract} "gtk" "gtk-2.22.1-dev-mensia.zip" "https://www.dropbox.com/s/xj7sz27tmb2et73/gtk-2.22.1-dev-mensia.zip?dl=1"
	${DownloadFromHTTPSAndExtract} "gtk" "gtk-2.22.1-runtime-mensia.zip" "https://www.dropbox.com/s/a5bmlqfukh8z5js/gtk-2.22.1-runtime-mensia.zip?dl=1"

	FileOpen $0 "$INSTDIR\gtk\lib\pkgconfig\gtk+-win32-2.0.pc" w
	FileWrite $0 "prefix=$INSTDIR\gtk$\r$\n"
	FileWrite $0 "exec_prefix=$${prefix}$\r$\n"
	FileWrite $0 "libdir=$${exec_prefix}/lib$\r$\n"
	FileWrite $0 "includedir=$${prefix}/include$\r$\n"
	FileWrite $0 "target=win32$\r$\n"
	FileWrite $0 "$\r$\n"
	FileWrite $0 "Name: GTK+$\r$\n"
	FileWrite $0 "Description: GTK+ Graphical UI Library ($${target} target)$\r$\n"
	FileWrite $0 "Version: 2.22.1$\r$\n"
	FileWrite $0 "Requires: gdk-$${target}-2.0 atk cairo gdk-pixbuf-2.0 gio-2.0$\r$\n"
	FileWrite $0 "Libs: -L$${libdir} -lgtk-$${target}-2.0$\r$\n"
	FileWrite $0 "Cflags: -I$${includedir}/gtk-2.0$\r$\n"
	FileClose $0

	FileOpen $0 "$INSTDIR\gtk\lib\pkgconfig\gthread-2.0.pc" w
	FileWrite $0 "prefix=$INSTDIR\gtk$\r$\n"
	FileWrite $0 "exec_prefix=$${prefix}$\r$\n"
	FileWrite $0 "libdir=$${exec_prefix}/lib$\r$\n"
	FileWrite $0 "includedir=$${prefix}/include$\r$\n"
	FileWrite $0 "$\r$\n"
	FileWrite $0 "Name: GThread$\r$\n"
	FileWrite $0 "Description: Thread support for GLib$\r$\n"
	FileWrite $0 "Requires: glib-2.0$\r$\n"
	FileWrite $0 "Version: 2.26.0$\r$\n"
	FileWrite $0 "Libs: -L$${libdir} -lgthread-2.0$\r$\n"
	FileWrite $0 "Cflags:$\r$\n"
	FileClose $0

	;Default them : MS windows
	CopyFiles "$INSTDIR\dependencies\gtk\share\themes\MS-Windows\gtk-2.0\gtkrc" "$INSTDIR\dependencies\gtk\etc\gtk-2.0\"

SectionEnd

;##########################################################################################################################################################

Section "GTK+ themes"

	SetOutPath "$INSTDIR"
	CreateDirectory "$INSTDIR\arch"

	${DownloadFromHTTPSAndExtract} "gtk" "gtk-themes-2009.09.07.zip" "https://www.dropbox.com/s/h56e2kl4n7t8iwg/gtk-themes-2009.09.07.zip?dl=1"

	; FileOpen $0 "$INSTDIR\gtk\etc\gtk-2.0\gtkrc" w
	; FileWrite $0 "gtk-theme-name = $\"Redmond$\"$\r$\n"
	; FileWrite $0 "style $\"user-font$\"$\r$\n"
	; FileWrite $0 "{$\r$\n"
	; FileWrite $0 "	font_name=$\"Sans 8$\"$\r$\n"
	; FileWrite $0 "}$\r$\n"
	; FileWrite $0 "widget_class $\"*$\" style $\"user-font$\"$\r$\n"
	; FileClose $0

SectionEnd

;##########################################################################################################################################################

Section "eigen"

	SetOutPath "$INSTDIR"
	CreateDirectory "$INSTDIR\arch"

	${DownloadFromHTTPSAndExtract} "eigen" "eigen-3.2.0-dev.zip" "https://www.dropbox.com/s/yzkqhslx74nvw93/eigen-3.2.0-dev.zip?dl=1"

SectionEnd

;##########################################################################################################################################################

Section "VRPN"

	SetOutPath "$INSTDIR"
	CreateDirectory "$INSTDIR\arch"

	${DownloadFromHTTPSAndExtract} "vrpn" "vrpn-7.26-$suffix-dev.zip" "https://www.dropbox.com/s/kc6n865dviys1qz/vrpn-7.26-vs100-dev.zip?dl=1"
	${DownloadFromHTTPSAndExtract} "vrpn" "vrpn-7.26-runtime.zip" "https://www.dropbox.com/s/5edlk5c3lofqtz6/vrpn-7.26-runtime.zip?dl=1"

	FileOpen $0 "$EXEDIR\win32-dependencies.cmd" a
	FileSeek $0 0 END
	FileWrite $0 "SET VRPNROOT=$INSTDIR\vrpn$\r$\n"
	FileWrite $0 "SET PATH=%VRPNROOT%\bin;%PATH%$\r$\n"
	FileClose $0

SectionEnd

;##########################################################################################################################################################

Section "pthreads"

	SetOutPath "$INSTDIR"
	CreateDirectory "$INSTDIR\arch"

	${DownloadFromHTTPSAndExtract} "pthreads" "pthreads-2.8.0-dev.zip" "https://www.dropbox.com/s/fpqua56pybeyguh/pthreads-2.8.0-dev.zip?dl=1"
	${DownloadFromHTTPSAndExtract} "pthreads" "pthreads-2.8.0-runtime.zip" "https://www.dropbox.com/s/5l9rt4ca813o5bz/pthreads-2.8.0-runtime.zip?dl=1"

SectionEnd

;##########################################################################################################################################################

Section "OpenAL"

	SetOutPath "$INSTDIR"
	CreateDirectory "$INSTDIR\arch"

	${DownloadFromHTTPSAndExtract} "openal" "openal-1.1-dev.zip" "https://www.dropbox.com/s/zz00r4mqkjs8ovu/openal-1.1-dev.zip?dl=1"
	${DownloadFromHTTPSAndExtract} "openal" "openal-1.1-runtime.zip" "https://www.dropbox.com/s/fk6rq292pkh5veh/openal-1.1-runtime.zip?dl=1"

SectionEnd

;##########################################################################################################################################################

Section "Alut"

	SetOutPath "$INSTDIR"
	CreateDirectory "$INSTDIR\arch"

	${DownloadFromHTTPSAndExtract} "freealut" "freealut-1.1.0-bin-dev.zip" "https://www.dropbox.com/s/hb8pa6k5mfz4mlw/freealut-1.1.0-bin-dev.zip?dl=1"
	${DownloadFromHTTPSAndExtract} "freealut" "freealut-1.1.0-bin-runtime.zip" "https://www.dropbox.com/s/7dif56pgr9rq4mp/freealut-1.1.0-bin-runtime.zip?dl=1"

SectionEnd

;##########################################################################################################################################################

Section "Ogg"

	SetOutPath "$INSTDIR"
	CreateDirectory "$INSTDIR\arch"

	; ${DownloadFromHTTPAndExtract} "libogg" "libogg-1.2.1-$suffix-dev.zip" "http://openvibe.inria.fr/dependencies/win32/libogg-1.2.1-$suffix-dev.zip"
	; ${DownloadFromHTTPAndExtract} "libogg" "libogg-1.2.1-$suffix-runtime.zip" "http://openvibe.inria.fr/dependencies/win32/libogg-1.2.1-$suffix-runtime.zip"
	${DownloadFromHTTPSAndExtract} "libogg" "libogg-1.2.1-vs100-dev.zip" "https://www.dropbox.com/s/62xcsclod4hwhs5/libogg-1.2.1-vs100-dev.zip?dl=1"
	${DownloadFromHTTPSAndExtract} "libogg" "libogg-1.2.1-vs100-runtime.zip" "https://www.dropbox.com/s/21vuznqdvyuvud6/libogg-1.2.1-vs100-runtime.zip?dl=1"

SectionEnd

;##########################################################################################################################################################

Section "Vorbis"

	SetOutPath "$INSTDIR"
	CreateDirectory "$INSTDIR\arch"

	; ${DownloadFromHTTPAndExtract} "libvorbis" "libvorbis-1.3.2-$suffix-dev.zip"
	; ${DownloadFromHTTPAndExtract} "libvorbis" "libvorbis-1.3.2-$suffix-runtime.zip"
	${DownloadFromHTTPSAndExtract} "libvorbis" "libvorbis-1.3.2-vs100-dev.zip" "https://www.dropbox.com/s/3jil80p7xbs3xlo/libvorbis-1.3.2-vs100-dev.zip?dl=1"
	${DownloadFromHTTPSAndExtract} "libvorbis" "libvorbis-1.3.2-vs100-runtime.zip" "https://www.dropbox.com/s/99u1utni1m4dtv2/libvorbis-1.3.2-vs100-runtime.zip?dl=1"

SectionEnd


;##########################################################################################################################################################

Section "TVicPort"

	SetOutPath "$INSTDIR"
	CreateDirectory "$INSTDIR\arch"

	; TVicPort 5.0.2.1
	${DownloadFromHTTPSAndExtract} "tvicport" "tvicport-5.0.2.1-devel.zip" "https://www.dropbox.com/s/tnonzk6ii9e27n1/tvicport-5.0.2.1-devel.zip?dl=1"
	${DownloadFromHTTPSAndExtract} "tvicport" "tvicport-5.0.2.1-runtime.zip" "https://www.dropbox.com/s/po6v2qxzji2dcfz/tvicport-5.0.2.1-runtime.zip?dl=1"

SectionEnd

;##########################################################################################################################################################

; Vanilla 5.1.4, simply repackaged, with DBI and Luacom
Section "Lua"

	SetOutPath "$INSTDIR"
	CreateDirectory "$INSTDIR\arch"

	${DownloadFromHTTPSAndExtract} "lua" "lua-5.1.4-mensia-r2.zip" "https://www.dropbox.com/s/95dypsd3d97lyp3/lua-5.1.4-mensia-r2.zip?dl=1"

;	FileWrite $0 "SET LUA_CPATH=?.dll;%OV_DEP_LUA%\bin\clibs\?.dll$\r$\n"
;	FileWrite $0 "SET LUA_PATH=?.lua;%OV_DEP_LUA%\bin\lua\?.lua$\r$\n"

SectionEnd

;##########################################################################################################################################################

Section "Zip"

	SetOutPath "$INSTDIR"
	CreateDirectory "$INSTDIR\arch"

	${DownloadFromHTTPSAndExtract} "zip" "zip-3.0-mensia.zip" "https://www.dropbox.com/s/50ooy3cgcfpxwae/zip-3.0-mensia.zip?dl=1"

SectionEnd

;##########################################################################################################################################################

Section "Third-Parties-Dependencies"

	SetOutPath "$INSTDIR"
	CreateDirectory "$INSTDIR\arch"

	${DownloadFromHTTPSAndExtract} "third-parties-dependencies" "third-parties-dependencies-2.5.11.zip" "https://www.dropbox.com/s/ors02e81xj3ivdf/third-parties-dependencies-2.5.11.zip?dl=1"

SectionEnd

;##########################################################################################################################################################

Section "ninja"

	SetOutPath "$INSTDIR"
	CreateDirectory "$INSTDIR\arch"

	${DownloadFromHTTPSAndExtract} "ninja" "ninja-1.4.0.zip" "https://www.dropbox.com/s/jdgswsl5lvcokrp/ninja-1.4.0.zip?dl=1"

SectionEnd

;##########################################################################################################################################################

Section "Plink"

	SetOutPath "$INSTDIR"
	CreateDirectory "$INSTDIR\arch"

	${DownloadFromHTTPSAndExtract} "plink" "plink-0.62.0.0.zip" "https://www.dropbox.com/s/jl9c2atisk18j0w/plink-0.62.0.0.zip?dl=1"

SectionEnd

;##########################################################################################################################################################

Section "Curl"

	SetOutPath "$INSTDIR"
	CreateDirectory "$INSTDIR\arch"
	${DownloadFromHTTPS}                  "curl-7.19.3-runtime.zip" "https://www.dropbox.com/s/794cnlm6563ogqk/curl-7.19.3-runtime.zip?dl=1"
	${DownloadFromHTTPSAndExtract} "curl" "curl-7.19.3-dev.zip"     "https://www.dropbox.com/s/f2eu3tamf9cbgbu/curl-7.19.3-dev.zip?dl=1"


SectionEnd

;##########################################################################################################################################################

Section "Electron"

	SetOutPath "$INSTDIR"
	CreateDirectory "$INSTDIR\arch"
	${DownloadFromHTTPSAndExtract} "electron" "electron-v0.29.2-win32-ia32.zip" "https://www.dropbox.com/s/b9os1u2ppz1hqyx/electron-v0.29.2-win32-ia32.zip?dl=1"

SectionEnd

;##########################################################################################################################################################

Section "NDK"

	SetOutPath "$INSTDIR"
	CreateDirectory "$INSTDIR\arch"

	IfFileExists "arch\ndk.zip" +1
		DetailPrint "ndk.zip was found. Extract it."
		${ExtractZip} "ndk.zip" "ndk"
SectionEnd


;##########################################################################################################################################################

Section "PortAudio"

	SetOutPath "$INSTDIR"
	CreateDirectory "$INSTDIR\arch"

	; PortAudio 19
	${DownloadFromHTTPSAndExtract} "portaudio" "portaudio-19-devel.zip" "https://www.dropbox.com/s/cd46cnh062xw3cw/portaudio-19-devel.zip?dl=1"
	${DownloadFromHTTPSAndExtract} "portaudio" "portaudio-19-runtime.zip" "https://www.dropbox.com/s/vjn0g3gfmfn2s9m/portaudio-19-runtime.zip?dl=1"

SectionEnd

;##########################################################################################################################################################

Section "SFML"

	SetOutPath "$INSTDIR"
	CreateDirectory "$INSTDIR\arch"

	${DownloadFromHTTPSAndExtract} "sfml" "sfml-2.3.2.zip" "https://www.dropbox.com/s/74w38z48ly2zck8/sfml-2.3.2.zip?dl=1"

SectionEnd

;##########################################################################################################################################################

Section "SFML.net"

	SetOutPath "$INSTDIR"
	CreateDirectory "$INSTDIR\arch"

	; SFML.net 2.1.0.0 5.0.2.1
	${DownloadFromHTTPSAndExtract} "sfml.net" "sfml.net-2.1.0.0-runtime.zip" "https://www.dropbox.com/s/1kpz4quem09k3rs/sfml.net-2.1.0.0-runtime.zip?dl=1"

SectionEnd


;##########################################################################################################################################################

Section "FTD2XX"
	SetOutPath "$INSTDIR"
	CreateDirectory "$INSTDIR\arch"

	; FTD2XX v2.12.10
	${DownloadFromHTTPSAndExtract} "ftd2xx" "cdm-v2.12.10.zip" "https://www.dropbox.com/s/d1tcd0oi6jzqjou/CDM%20v2.12.10%20WHQL%20Certified.zip?dl=1"

SectionEnd


;##########################################################################################################################################################

Section "Signals" SEC_Signals

	SetOutPath "$INSTDIR"
	CreateDirectory "$INSTDIR\arch"
	CreateDirectory "$INSTDIR\signals"

	; NeuroRT Showcases sample signals
	${DownloadFromHTTPSAndExtract} "signals" "showcases-signals-2.5.0.0.zip" "https://www.dropbox.com/s/21cgu8iqt36r30m/showcases-signals-2.5.0.0.zip?dl=1"

SectionEnd

;##########################################################################################################################################################

Section "Python 3"

	SetOutPath "$INSTDIR"
	CreateDirectory "$INSTDIR\arch"
	CreateDirectory "$INSTDIR\python"

	; Python 3.5
	${DownloadFromHTTPSAndExtract} "python" "python-3.5.2-32-devel.zip" "https://www.dropbox.com/s/mcieuobq6jzi86b/python-3.5.2-32-devel.zip?dl=1"

SectionEnd

;##########################################################################################################################################################

SectionGroup "Release tools"

	Section /o "First Section" SEC_Release_Tools_A
		DetailPrint "This section is here to help selecting/unselecting the whole section 'Release Tools'"
	SectionEnd

	Section /o "NSIS" SEC_Release_Tools_NSIS

		SetOutPath "$INSTDIR"
		CreateDirectory "$INSTDIR\arch"

		${DownloadFromHTTPS} "nsis-3.0b1-setup.exe" "https://www.dropbox.com/s/xrom7069567rzt6/nsis-3.0b1-setup.exe?dl=1"
		ExecWait '"arch\nsis-3.0b1-setup.exe" /S'

		IfFileExists  "$PROGRAMFILES\NSIS" nsis_installed
			MessageBox MB_OK "NSIS not found, NsProcess plugin won't be installed." /SD IDOK
			Goto nsis_not_installed

		nsis_installed:

		DetailPrint "Install plugin NSIS - AccessControl plugin"
		${DownloadFromHTTPS} "nsis-plugin-AccessControl.zip" "https://www.dropbox.com/s/j44949k4c7vk481/nsis-plugin-AccessControl.zip?dl=1"
		ZipDLL::extractall "arch\nsis-plugin-AccessControl.zip" "$PROGRAMFILES\NSIS"

		DetailPrint "Install plugin NSIS - nsisFirewall plugin"
		${DownloadFromHTTPS} "nsis-plugin-Firewall.zip" "https://www.dropbox.com/s/oqe2ivcnmgjkuf8/nsis-plugin-Firewall.zip?dl=1"
		ZipDLL::extractall "arch\nsis-plugin-Firewall.zip" "$PROGRAMFILES\NSIS"

		DetailPrint "Install plugin NSIS - NsDialogs_createTextMultiline plugin"
		${DownloadFromHTTPS} "NsDialogs_createTextMultiline.zip" "https://www.dropbox.com/s/rfs19ora7slfxf3/NsDialogs_createTextMultiline.zip?dl=1"
		ZipDLL::extractall "arch\NsDialogs_createTextMultiline.zip" "$PROGRAMFILES\NSIS\Include"

		DetailPrint "Install plugin NSIS - inetc plugin"
		${DownloadFromHTTPS} "nsis-plugin-Inetc.zip" "https://www.dropbox.com/s/zk5hspft5obm7mn/nsis-plugin-Inetc.zip?dl=1"
		ZipDLL::extractall "arch\nsis-plugin-Inetc.zip" "$PROGRAMFILES\NSIS"

		DetailPrint "Install plugin NSIS - ZipDLL plugin"
		${DownloadFromHTTPS} "nsis-plugin-ZipDLL.zip" "https://www.dropbox.com/s/skealnf9sysyi3f/nsis-plugin-ZipDLL.zip?dl=1"
		ZipDLL::extractall "arch\nsis-plugin-ZipDLL.zip" "$PROGRAMFILES\NSIS"

		DetailPrint "Install plugin NSIS - NsProcess plugin"
		${DownloadFromHTTPS} "nsis-plugin-NsProcess.zip" "https://www.dropbox.com/s/cw7jc5x2tmx35cg/nsis-plugin-NsProcess.zip?dl=1"
		ZipDLL::extractall "arch\nsis-plugin-NsProcess.zip" "$PROGRAMFILES\NSIS"

		nsis_not_installed:

	SectionEnd

	;##########################################################################################################################################################

	Section /o "Documentation" SEC_Release_Tools_Documentation

		SetOutPath "$INSTDIR"
		CreateDirectory "$INSTDIR\arch"
		CreateDirectory "$INSTDIR\doc"

		${DownloadDocFromHTTPS} "doc\NeuroRT Redistributables - Developer Manual- v2.5.0.pdf" "https://www.dropbox.com/s/k6yat34l9drc4g9/NeuroRT%20Redistributables%20-%20Developer%20Manual%20v2.5.0.pdf?dl=1"

		; NeuroRT Drivers
		${DownloadFromHTTPSAndExtract} "doc" "doc-drivers.zip" "https://www.dropbox.com/s/5xedvb2njw3lw88/doc-drivers.zip?dl=1"

		; default documentation
		; default documentation (OEM Mensia)
		${DownloadDocFromHTTPS} "doc\NeuroRT Studio v2.5.0 - User Manual.pdf"  "https://www.dropbox.com/s/1uu8kndlyf0ecxl/NeuroRT%20Studio%20v2.5.0%20-%20User%20Manual.pdf?dl=1"

		; NeuroRT Box Plugins
		${DownloadFromHTTPSAndExtract} "doc" "doc-box-plugins.zip" "https://www.dropbox.com/s/jev4y2j7azfdas5/neurort-box-plugins-documentation.zip?dl=1"

		; ADHD At Home
		${DownloadFromHTTPSAndExtract} "doc" "doc-adhd-at-home-1.0.0.0.zip" "https://www.dropbox.com/s/1ghbmjjon03mweq/doc-adhd-at-home-1.0.0.0.zip?dl=1"

	SectionEnd

	;##########################################################################################################################################################

	Section /o "PortQry" SEC_Release_Tools_PortQry

		SetOutPath "$INSTDIR"
		CreateDirectory "$INSTDIR\arch"

		${DownloadFromHTTPSAndExtract} "portqry" "portqry.zip" "https://www.dropbox.com/s/arb781y1x6uven2/portqry.zip?dl=1"

	SectionEnd

	;##########################################################################################################################################################

	Section /o "Visual Redistributable Packages" SEC_Release_Tools_Redist

		SetOutPath "$INSTDIR"
		CreateDirectory "$INSTDIR\arch"

		; We install VC 2010 SP1 redist as DLL files.
		; the dll must be packaged with the application afterwards
		${DownloadFromHTTPSAndExtract} "vcredist" "vcredist-2010-sp1.zip" "https://www.dropbox.com/s/qfx5av3rv26evz5/vcredist-2010-sp1.zip?dl=1"
		; Debug DLL for developers
		${DownloadFromHTTPSAndExtract} "vcredist" "vcredist-2010-sp1-debug.zip" "https://www.dropbox.com/s/t38fky2fcrveagi/vcredist-2010-sp1-debug.zip?dl=1"

		; Visual Studio 2013
		${DownloadFromHTTPSAndExtract} "vcredist" "vcredist-2013.zip" "https://www.dropbox.com/s/5jme32f6vjq31yw/vcredist-2013.zip?dl=1"
		; Debug DLL for developers
		${DownloadFromHTTPSAndExtract} "vcredist" "vcredist-2013-debug.zip" "https://www.dropbox.com/s/w6vc98jkfbzcyyg/vcredist-2013-debug.zip?dl=1"


	SectionEnd

	;##########################################################################################################################################################

	Section /o "Packaging - ADHD At Home" SEC_Release_Tools_ADHD

		SetOutPath "$INSTDIR"
		CreateDirectory "$INSTDIR\arch"

		${DownloadFromHTTPSAndExtract} "packaging" "packaging-adhd-at-home-1.0.0.0.zip" "https://www.dropbox.com/s/jkq44ytq618n0uq/packaging-adhd-at-home-1.0.0.0.zip?dl=1"

	SectionEnd

	Section /o "Last Section" SEC_Release_Tools_Z
		DetailPrint "This section is here to help selecting/unselecting the whole section 'Release Tools'"
	SectionEnd
SectionGroupEnd

;##########################################################################################################################################################

Section "Registration"

	${AddToSetEnv} "boost\bin"
	${AddToSetEnv} "cmake\bin"
	${AddToSetEnv} "curl"
	${AddToSetEnv} "ftd2xx\bin"
	; ${AddToSetEnv} "doc" ; no DLL
	; ${AddToSetEnv} "eigen" ; no DLL
	${AddToSetEnv} "electron"
	${AddToSetEnv} "expat\bin"
	${AddToSetEnv} "freealut\lib"
	${AddToSetEnv} "gtk\bin"
	${AddToSetEnv} "libmysql\bin"
	${AddToSetEnv} "libogg\win32\bin\debug"
	${AddToSetEnv} "libogg\win32\bin\release"
	${AddToSetEnv} "libvorbis\win32\bin\debug"
	${AddToSetEnv} "libvorbis\win32\bin\release"
	${AddToSetEnv} "lua\bin" ; Lua before Love2D in the path, so any code compiled with luac.exe would still run.
	${AddToSetEnv} "ninja"
	${AddToSetEnv} "openal\libs\win32"
	${AddToSetEnv} "plink"
	${AddToSetEnv} "portaudio\bin"
	${AddToSetEnv} "pthreads\lib"
	${AddToSetEnv} "sfml\bin"
	${AddToSetEnv} "sfml.net\lib"
	${AddToSetEnv} "tvicport\bin"
	${AddToSetEnv} "vcredist"
	; ${AddToSetEnv} "vrpn" ; no DLL
	${AddToSetEnv} "zip"
	${AddToSetEnv} "portqry\bin"

	; third party dependencies need to be individually added
	${AddToSetEnv} "third-parties-dependencies/ant eego"
	${AddToSetEnv} "third-parties-dependencies/biosemi activetwo/bin"
	${AddToSetEnv} "third-parties-dependencies/brainmaster discovery and atlantis/bin"
	${AddToSetEnv} "third-parties-dependencies/brainproducts actichamp/bin"
	${AddToSetEnv} "third-parties-dependencies/brainproducts firstamp/bin"
	${AddToSetEnv} "third-parties-dependencies/brain rhythm incorporation/bin"
	${AddToSetEnv} "third-parties-dependencies/emotiv epoc/bin"
	${AddToSetEnv} "third-parties-dependencies/gtec gusbamp/bin"
	${AddToSetEnv} "third-parties-dependencies/liblsl/bin"
	${AddToSetEnv} "third-parties-dependencies/micromed/bin"
	${AddToSetEnv} "third-parties-dependencies/mindmedia nexus32b/bin"
	${AddToSetEnv} "third-parties-dependencies/mitsar/bin"
	${AddToSetEnv} "third-parties-dependencies/neuroelectrics enobio3g/bin"
	${AddToSetEnv} "third-parties-dependencies/neurosky/bin"
	${AddToSetEnv} "third-parties-dependencies/smi-iviewng/bin"
	${AddToSetEnv} "third-parties-dependencies/smi-iviewx/bin"
	${AddToSetEnv} "third-parties-dependencies/tmsi/bin"
	${AddToSetEnv} "third-parties-dependencies/wearablesensing dsi"

	${AddToManifest} BOOST 1.55
	${AddToManifest} CMAKE 2.8.12.1
	${AddToManifest} CURL 7.19.3
	${AddToManifest} DOC 1.0.0
	${AddToManifest} EIGEN 3.2.0
	${AddToManifest} ELECTRON 0.29.2
	${AddToManifest} EXPAT 2.0.1
	${AddToManifest} FREEALUT 1.1
	${AddToManifest} FTD2XX 2.12.10
	${AddToManifest} GTK 2.22.1
	${AddToManifest} LIBMYSQL 10.0.13
	${AddToManifest} LIBOGG 1.2.1
	${AddToManifest} LIBVORBIS 1.3.2
	${AddToManifest} LUA 5.1.4
	${AddToManifest} NINJA 1.4.0
	${AddToManifest} NSIS 3.0b1
	${AddToManifest} OPENAL 1.1
	${AddToManifest} PLINK 0.62.0.0
	${AddToManifest} PORTQRY 1.0
	${AddToManifest} PORTAUDIO 19
	${AddToManifest} PTHREADS 2.8.0
	${AddToManifest} SIGNALS 2.5.0.0
	${AddToManifest} THIRDPARTIES 2.5.11
	${AddToManifest} TVICPORT 5.0.2.1
	${AddToManifest} VRPN 7.26
	${AddToManifest} ZIP 3.0

SectionEnd

;##########################################################################################################################################################

Section "Uninstall"

	RMDir /r "$INSTDIR\gtk"
	RMDir /r "$INSTDIR\boost"
	RMDir /r "$INSTDIR\electron"
	RMDir /r "$INSTDIR\expat"
	RMDir /r "$INSTDIR\cmake"
	RMDir /r "$INSTDIR\lua"
	RMDir /r "$INSTDIR\vrpn"
	RMDir /r "$INSTDIR\vcredist"
	RMDir /r "$INSTDIR\openal"
	RMDir /r "$INSTDIR\alut"
	RMDir /r "$INSTDIR\libogg"
	RMDir /r "$INSTDIR\libvorbis"
	RMDir /r "$INSTDIR\zip"
	RMDir /r "$INSTDIR\tmp"
	RMDir /r "$INSTDIR\pthreads"
	RMDir /r "$INSTDIR\ninja"
	RMDir /r "$INSTDIR\plink"
	RMDir /r "$INSTDIR\python"
	RMDir /r "$INSTDIR\sfml"
	RMDir /r "$INSTDIR\sfml.net"
	RMDir /r "$INSTDIR\tvciport"
	RMDir /r "$INSTDIR\portaudio"
	RMDir /r "$INSTDIR\third-parties-dependencies"

	RMDir /r "$INSTDIR\doc"

	Delete "$INSTDIR\..\scripts\win32-dependencies.cmd"

	Delete "$INSTDIR\Uninstall.exe"

	RMDir "$INSTDIR"

SectionEnd

;##########################################################################################################################################################

Function .onInit
	StrCpy $9 ${vs90}
	IfSilent 0 non_silent
		!insertmacro SelectSection $1
		${For} $1 ${SEC_Release_Tools_A} ${SEC_Release_Tools_Z}
			!insertmacro SelectSection $1
		${Next}
	non_silent:
FunctionEnd

Function .onSelChange
  !insertmacro StartRadioButtons $9
    !insertmacro RadioButton ${vs90}
    !insertmacro RadioButton ${vs100}
  !insertmacro EndRadioButtons
FunctionEnd
