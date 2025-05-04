# NymphCast Game Synchronisation #

This document covers how to set up and use the synchronisation mechanism for games and their save files with NymphCast. Specifically, it works with the NymphCast Media Server (NCMS) and the NymphCast Server (NCS) with the latter running in GUI mode. See the [main project Readme](README.md) for details on those components.

## Overview ##

NC Game Synchronisation (NCGS) targets emulated games, from a whole range of older and newer video game consoles and handhelds. Along with the installation of the 

## NCMS Setup ##

The NCGS feature on the NCMS side relies on a specific folder layout:

```
/
|-/games
	|-/nes
	|-/snes
	|-/n64
		|- system.ini
		|-/roms
		|	|- A.n64
		|	|- B.n64
		|
		|-/saves
			|- A.sav
			|- B.sav
```
			
The `system.ini` configuration file is a per-system configuration file with the following options:

- `name`		- The short system name. Also used for the theme and for scraping.
- `long_name`	- The long (decorative) name.
- `extensions`	- ROM extensions for this system.
- `launch_cmd`	- Command used to launch the emulator with a game.
- `theme`		- (Optional) theme name if different from `name`.

An example system.ini would look like this:

```
- - -
name = snes
long_name = Super Nintendo Entertainment System
extensions = smc,sfc,SMC,SFC
launch_cmd = retroarch -L ~/.config/retroarch/cores/snes9x_libretro.so %ROM%
theme = snes
- - -
```

## NCS Setup ##

Since NCGS relies on having the emulator software installed on the system the game is being run on, this has to be installed separately from NCS. The reference emulator software is RetroArch, which can be installed by following the [instructions](https://www.retroarch.com/index.php?page=platforms) on their website.

The NCGS feature also has to be enabled in the NCS configuration file: set `ncgs_enable` to `1`.

## NCS Usage ##

Ensure that NCS is started in GUI mode, either by using the GUI preset configuration file or editing the currently used configuration files.

When NCS starts in GUI mode, it will automatically obtain the game file info from the NCMS instance. It will synchronise local and remote files.
