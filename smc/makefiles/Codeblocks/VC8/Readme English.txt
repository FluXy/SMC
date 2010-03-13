Configuration Codeblocks 1.0, last change 14.1.2007

## Visual C++ must be installed.
## The SDK for Windows Applications must be installed.
## You need the VC6 - Development-Versions of the following libraries #########

SDL
SDL_image
SDL_mixer
SDL_ttf

- The include & lib has to be copied in the corresponding folders at Visual C++.


## Setup the SDK of CEGUI #########

- Use CEGUI_SDK 0.5.0-vc8
- For better oversight, unzip the include & lib into its own folder.

## Setup Boost ###############

- Download last stable Boost Release
- Build Boost Jam (Bjam) by executing tools\build\jam_src\build.bat
- Copy the bjam.exe to the boost root folder
- #define BOOST_ALL_DYN_LINK in boost/config/user.cpp for dynamic linking
- Open a Command prompt and execute vsvars32.bat
- Enter for only the filesystem debug lib :  bjam "-sBUILD=debug <runtime-link>dynamic <threading>multi" -sTOOLS=vc-8_0 --stagedir=. --with-filesystem
- Setup the include/lib folders

## Codeblocks has now to be set up, the following compiler options have to be activated (if not already)

/EHs
/EHa
/MD


## The compiler must be changed to Microsoft Visual C++ 2005 (if not already)

## Afterwards the include & lib for SDL, CEGUI and the Windows SDK has to be set up, then you can start SMC

## TIP: Set up your include & lib's under Settings / Compiler and Debugger (of course for VC++), then it works for every project (because of this no directories are entered in the project file)

________________

Why take Microsoft VC and not MinGW ?
- Because CEGUI is not yet compatible with MinGW. The application won't get linked.