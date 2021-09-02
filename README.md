# NymphCast MediaServer #

NymphCast MediaServer (NC-MS) is part of the [NymphCast project](https://github.com/MayaPosch/NymphCast). It enables the sharing of media files to NymphCast clients and servers on the network. Provided is a server application which can be installed on a system located on the same local network (LAN) as those servers and clients.

## Features ##


* Make media files in specific folders available for NymphCast clients.
* Enable the streaming of shared media files to specific NymphCast devices, or groups of devices.

## Status ##

NC-MS is - as specified on the central NymphCast project page - an Alpha-level project. This means that bugs are to be expected, features may be missing and no easy-to-use installers are provided as of yet.


## Getting Started ##

NC-MS has to be compiled from source. This requires that the following dependencies are installed:

* [NymphRPC](https://github.com/MayaPosch/NymphRPC)
* NymphCast client SDK (See NymphCast project page)
* LibPoco

With these installed along with a C++17 supporting compiler, clone the NC-MS repository to one's system and in the root folder of the project execute `make`. This compiles a binary called `nc_mediaserver` under `/bin`.

The binary supports the following flags:

```
Usage:
        nc_mediaserver <options>

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

On Windows, use regular Windows path, e.g. `D:\Media\Video`.

These shared folder are scanned recursively (including sub-folders) for media files (audio, video, images) based on their extensions. A full list of extensions can be found in [mimetype.cpp](src/mimetype.cpp).