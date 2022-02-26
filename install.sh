#!/bin/sh

# Installation script for a packaged version of NymphCast MedaServer (NCMS).
# To be used with the 'package' target of the NCMS Makefile: 
#	- 'make package' copies this installation script into the resulting .tar.gz package.
#	- After unpackaging the archive, running this script will install NCMS's components.

# Ensure we got all dependencies in place.
if [ -x "$(command -v apt)" ]; then
	sudo apt update
	sudo apt -y install libpoco-dev
elif [ -x "$(command -v apk)" ]; then
	sudo apk update
	sudo apk add poco-dev
elif [ -x "$(command -v pacman)" ]; then
	sudo pacman -Syy 
	sudo pacman -S --noconfirm --needed poco
fi


# Copy the files to their locations.
sudo cp -a lib/* /usr/lib/.
sudo install -d /usr/local/etc/nymphcast/
sudo install -m 755 bin/nymphcast_mediaserver /usr/local/bin/
sudo install -m 644 *.ini /usr/local/etc/nymphcast/

# Ask to install a system service to start NCMS automatically
read -p "Install systemd service for NymphCast MediaServer? [y/n] (default: n): " choice
if [ "$choice" = "y" ]; then
	echo "Installing systemd service..."
	sudo cp systemd/nymphcast_mediaserver.service /etc/systemd/system/nymphcast_mediaserver.service
	sudo systemctl enable nymphcast_mediaserver.service
else
	echo "Skipping system service installation..."
fi
