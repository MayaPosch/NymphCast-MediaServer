# NymphCast MediaServer #

NymphCast MediaServer (NCMS) is part of the [NymphCast project](https://github.com/MayaPosch/NymphCast). It enables the sharing of media files to NymphCast clients and servers on the network. Provided is a server application which can be installed on a system located on the same local network (LAN) as those servers and clients.


## Features ##


* Make media files in specific folders available for NymphCast clients.
* Enable the streaming of shared media files to specific NymphCast devices, or groups of devices.
* Playlist support: M3U playlists are shared and played back by NCMS.

## Quick Start ##

Download or clone the project, then after moving into the root project folder on supported platforms run:

```
./setup.sh
```

This will install any missing dependencies and compile NCMS for the host platform. 

After this NCMS can be installed on **Linux** platforms using:

```
sudo ./install_linux.sh
```

This will install NCMS and set up a Systemd/OpenRC service to auto-start NCMS on boot.

**Supported platforms** here are:

* Debian Linux and derivatives (Ubuntu, Mint, etc.).
* Arch Linux and derivatives (e.g. Manjaro).
* Alpine Linux and derivatives.
* Windows (in [MSYS2](http://msys2.org/) MinGW64 environment).
* Windows (MSVC)
* MacOS (using GCC toolchain & [Homebrew](https://brew.sh/)).

## Getting Started ##

NCMS has to be compiled from source. This requires that the following dependencies are installed:

* [NymphRPC](https://github.com/MayaPosch/NymphRPC)
* NymphCast client SDK (See NymphCast project page)
* LibPoco

With these installed along with a C++17 supporting compiler, clone the NCMS repository to one's system and in the root folder of the project execute `make`. This compiles a binary called `nymphcast_mediaserver` under `/bin/<toolchain>`.

## MSVC Support ##

Automated building using MSVC (2017, 2019 or 2022) is supported with dependencies provided by [vcpkg](https://vcpkg.io). Within the root folder of the project, execute the provided bat file in a native x64 MSVC shell:

`Setup-NMake-vcpkg.bat`

To clean intermediate build files:

`Setup-NMake-vcpkg.bat clean`

With the [InnoSetup](https://jrsoftware.org/isinfo.php) executable files in the system path, a Windows installer can be created using:

`Setup-NMake-vcpkg.bat package`


## Running NCMS ##

The binary supports the following flags:

```
Usage:
        nymphcast_mediaserver <options>

Options:
-h      --help                  Get this help message.
-c      --configuration         Path to configuration file.
-f      --folders               Path to folder list file.
-v      --version               Output the NymphCast version and exit.
```

The configuration file is an INI-format file (see sample [`folders.ini`](folders.ini) in the project), with the following layout:

```
[Audio]
path=audio

[Video]
path=video
```

Each section specifies the name of the collection, with the path variable defining the directory path (either relative to the media server binary or as absolute path) where the shared media files are located. 

On Windows, use a regular Windows path, e.g. `D:\Media\Video`.

These shared folder are scanned recursively (including sub-folders) for media files (audio, video, images) based on their extensions. A full list of extensions can be found in [mimetype.cpp](src/mimetype.cpp).