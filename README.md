# NeuroRT Studio

This repository contains the NeuroRT Studio based on Certivibe kernel. It also contains various visualization plugins.

## Build

In order to compile this project you will need to have the openvibe-sdk dependency compiled somewhere. You can install the last released version 
via dependencies or use a folder compiled locally folder.

Let us call this path `PATH_CERTIVIBE`. The other requirement is to know where Certivibe dependencies can be found, let us call this path 
`PATH_CERTIVIBE_DEPENDENCIES`. Finally, let us call the location of the source code `PATH_STUDIO_SOURCE`.

### Pre-requisites

In order to build Designer, you need a compiler installed. On Windows you need to have Visual Studio 2013 installed, the build system is based on CMake and Ninja, 
if you don't want to install it on your system, then you can unzip it locally with command by:

Going to `PATH_STUDIO_SOURCE\scripts` and run:

```
    powershell.exe -NoExit -NoProfile -ExecutionPolicy Bypass -File windows-install-dependencies.ps1 -dependencies_file PATH_STUDIO_SOURCE\scripts\windows-build-tools.txt -data_type dependencies -dest_dir PATH_STUDIO_SOURCE\dependencies
```

### Use last released version of openvibe-sdk

Go to `PATH_STUDIO_SOURCE\scripts` and run:

```
    powershell.exe -NoExit -NoProfile -ExecutionPolicy Bypass -File windows-install-dependencies.ps1 -dependencies_file PATH_STUDIO_SOURCE\scripts\windows-dependencies-sdk.txt -data_type dependencies -dest_dir PATH_STUDIO_SOURCE\dependencies
```

This script will download and unzip openvibe-sdk Debug and Release version under respectively `PATH_STUDIO_SOURCE/dependencies/openvibe-sdk-${build_type}` (${build_type} being debug or release)
folders. In this configuration, you won't need to overwrite `PATH_CERTIVIBE` and `PATH_CERTIVIBE_DEPENDENCIES`.

### Use local version of openvibe-sdk

By default the scripts expects the SDK to be in `PATH_STUDIO_SOURCE/dependencies/openvibe-sdk-${build_type}` (${build_type} being debug or release) and 
certivibe dependencies to be in packet.
If you use another repository, you need to set 

### Windows

#### Installing Studio Dependencies

First, install Studio dependencies.

Go to `PATH_STUDIO_SOURCE\scripts` and run:

```
    powershell.exe -NoExit -NoProfile -ExecutionPolicy Bypass -File windows-install-dependencies.ps1 -dependencies_file PATH_STUDIO_SOURCE\scripts\windows-dependencies.txt -data_type dependencies -dest_dir PATH_STUDIO_SOURCE\dependencies
```

This will install Studio dependencies into `PATH_STUDIO_SOURCE\dependencies`

#### Compile the source code via the script

The build script can be found in `PATH_STUDIO_SOURCE\scripts`

To build Studio in Release mode run:

     windows-build.cmd --sdk PATH_CERTIVIBE --dep PATH_CERTIVIBE_DEPENDENCIES

#### Creating a Visual Studio project

A Visual Studio project can be created using scripts. A generator can be found in the `PATH_STUDIO_SOURCE\scripts` folder.

    windows-generate-vs-project.cmd --sdk "PATH_CERTIVIBE" --dep "PATH_CERTIVIBE_DEPENDENCIES"

In order to open the visual studio with the correct paths:

    windows-launch-visual-studio.cmd --sdk "PATH_CERTIVIBE"

Note that currently, only building the project in Release mode is supported if you are using Visual Studio.

### Linux

#### Installing Studio Dependencies

This installation guide supposes that you have already installed the CertiViBE dependencies.

Go to `PATH_STUDIO_SOURCE/scripts` and run

    perl linux-install-dependencies.pl
    
You will be asked for your root password which you have to grant to the script.

#### Compile the source code via the script

The build script is in `PATH_STUDIO_SOURCE/scripts`

Run it as so:

    ./unix-build --sdk=PATH_CERTIVIBE
    
This will build and install Studio in release mode into `PATH_STUDIO_SOURCE/build/dist`
