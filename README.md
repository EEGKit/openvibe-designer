# NeuroRT Studio

This repository contains the NeuroRT Studio based on Certivibe kernel. It also contains various visualization plugins.

## Installation

In order to be able to compile this project you will need to have the Certivibe project compiled somewhere, let us call this path `PATH_CERTIVIBE`. The other requirement is to know where Certivibe dependencies can be found, let us call this path `PATH_CERTIVIBE_DEPENDENCIES`. Finally, let us call the location of the source code `PATH_STUDIO_SOURCE`.

By default the scripts expects the SDK to be in `PATH_STUDIO_SOURCE/dependencies/certivibe` and certivibe dependencies to be in `PATH_STUDIO_SOURCE/dependencies/certivibe-dependencies`

If you wish to build the Studio with complete debugging support, the default location for a debug build of Cerrtivibe is in `PATH_STUDIO_SOURCE/dependencies/certivibe-debug`

### Windows

#### Installing Studio Dependencies

First, install Studio dependencies.

Go to `PATH_STUDIO_SOURCE\scripts` and run

    powershell.exe -NoExit -NoProfile -ExecutionPolicy Bypass -File windows-install-dependencies.ps1 -dependencies_file PATH_STUDIO_SOURCE\scripts\windows-dependencies.txt -data_type dependencies -dest_dir PATH_STUDIO_SOURCE\dependencies

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
