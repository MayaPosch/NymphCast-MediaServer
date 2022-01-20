#!/bin/sh

# NymphCast installer for the Linux platform.
if [ "$(uname -s)" = "Linux" ]; then
	echo "Detected Linux system. Proceeding with installation..."
else
	echo "This installer requires a Linux system. Exiting..."
	exit
fi

# Requires that the binaries have been compiled with the 'setup.sh' script first.
PLATFORM=`g++ -dumpmachine`
if [ -f "src/server/bin/${PLATFORM}/nymphcast_server" ]; then
	echo "NymphCast Server binary found, skipping compilation..."
else
	echo "Compiling NymphCast MediaServer..."
	./setup.sh
fi

# Copy files to the target folders.
sudo make install

# Install systemd or openrc service.
if [ -d "/run/systemd/system" ]; then
	echo "Installing systemd service..."
	sudo make install-systemd
	sudo systemctl enable nymphcast.service
else
	echo "Installing OpenRC service..."
	sudo make install-openrc
fi

# Inform about the folder configuration file.
echo "Edit /usr/local/etc/nymphcast/folders.ini to add shared folders."

echo "Installation done."
