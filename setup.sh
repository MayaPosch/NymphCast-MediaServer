#!/bin/sh
# Compilation script for the NymphCast MediaServer
# Installs needed dependencies and compiles the project.

# Install the dependencies.
PLATFORM="unknown"
case "$(uname -s)" in
	Darwin)
		echo 'Mac OS X'
		PLATFORM="macos"
		if [ -x "$(command -v brew)" ]; then
			brew update
			brew install poco 
		fi
		;;

	Linux)
		echo 'Linux'
		PLATFORM="linux"
		if [ -x "$(command -v apt)" ]; then
			sudo apt update
			sudo apt -y install git g++ libpoco-dev
		elif [ -x "$(command -v apk)" ]; then
			sudo apk update
			sudo apk add poco-dev
		elif [ -x "$(command -v pacman)" ]; then
			sudo pacman -Syy 
			sudo pacman -S --noconfirm --needed git poco
		fi
		;;

	CYGWIN*|MINGW32*|MSYS*|MINGW*)
		echo 'MS Windows/MinGW'
		PLATFORM="mingw"
		if [ -x "$(command -v pacman)" ]; then
			pacman -Syy 
			pacman -S --noconfirm --needed git mingw-w64-x86_64-poco
		fi
		;;

	*)
		echo 'Unsupported OS'
		exit
		;;
esac


if [ -n "${UPDATE}" ]; then
	if [ "${PLATFORM}" == "linux" ]; then
		if [ -f "/usr/local/lib/libnymphrpc.a" ]; then
			sudo rm /usr/local/lib/libnymphrpc.*
			sudo rm -rf /usr/local/include/nymph
		fi
	elif [ "${PLATFORM}" == "mingw" ]; then
		if [ -f "/mingw64/lib/libnymphrpc.a" ]; then
			rm /mingw64/lib/libnymphrpc.a
		fi
	fi
fi

if [ -f "/usr/lib/libnymphrpc.so" ]; then
	echo "NymphRPC dynamic library found in /usr/lib. Skipping installation."
elif [ -f "/mingw64/lib/libnymphrpc.so" ]; then
	echo "NymphRPC dynamic library found in /mingw64/lib. Skipping installation."
else
	# Obtain current version of NymphRPC
	git clone --depth 1 https://github.com/MayaPosch/NymphRPC.git
	
	# Build NymphRPC and install it.
	echo "Installing NymphRPC..."
	make -C NymphRPC/ lib
	if [ "${PLATFORM}" == "mingw" ]; then
		make -C NymphRPC/ install
	else
		sudo make -C NymphRPC/ install
	fi
fi

# Remove NymphRPC folder.
rm -rf NymphRPC

# Build NymphCast client library.
#make -C src/client_lib/ clean
#make -C src/client_lib/
if [ -f "/usr/lib/libnymphcast.so" ]; then
	echo "LibNymphCast dynamic library found in /usr/lib. Skipping installation."
elif [ -f "/mingw64/lib/libnymphcast.so" ]; then
	echo "LibNymphCast dynamic library found in /mingw64/lib. Skipping installation."
else
	# Obtain current version of LibNymphCast
	git clone --depth 1 https://github.com/MayaPosch/libnymphcast.git
	
	# Build libnymphcast and install it.
	echo "Installing LibNymphCast..."
	make -C libnymphcast/ lib
	if [ "${PLATFORM}" == "mingw" ]; then
		make -C libnymphcast/ install
	else 
		sudo make -C libnymphcast/ install
	fi
fi

# Remove libnymphcast folder
rm -rf libnymphcast

# Build the server.
make

if [ ! -z "${PACKAGE}" ]; then
	# Packaging step.
fi
