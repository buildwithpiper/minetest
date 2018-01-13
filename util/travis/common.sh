#!/bin/bash -e

set_linux_compiler_env() {
	if [[ "${COMPILER}" == "gcc-5.1" ]]; then
		export CC=gcc-5.1
		export CXX=g++-5.1
	elif [[ "${COMPILER}" == "gcc-6" ]]; then
		export CC=gcc-6
		export CXX=g++-6
	elif [[ "${COMPILER}" == "gcc-7" ]]; then
		export CC=gcc-7
		export CXX=g++-7
	elif [[ "${COMPILER}" == "clang-3.6" ]]; then
		export CC=clang-3.6
		export CXX=clang++-3.6
	elif [[ "${COMPILER}" == "clang-5.0" ]]; then
		export CC=clang-5.0
		export CXX=clang++-5.0
	elif [[ "${COMPILER}" == "arm-linux-gnueabihf-gcc" ]]; then
		export CC=arm-linux-gnueabihf-gcc
		export CXX=arm-linux-gnueabihf-gcc
	fi
}

# Linux build only
install_linux_deps() {
	sudo apt-get update
	sudo apt-get install libirrlicht-dev cmake libbz2-dev libpng12-dev \
		libjpeg-dev libxxf86vm-dev libgl1-mesa-dev libsqlite3-dev \
		libhiredis-dev libogg-dev libgmp-dev libvorbis-dev libopenal-dev \
		gettext libpq-dev libleveldb-dev
}


# Raspbian build only
install_raspbian_deps() {
	wget https://archive.raspbian.org/raspbian.public.key -O - | sudo apt-key add -

	echo 'deb http://archive.raspbian.org/raspbian wheezy main' | sudo tee -a /etc/apt/sources.list
	echo 'deb-src http://archive.raspbian.org/raspbian wheezy main' | sudo tee -a /etc/apt/sources.list

	git clone https://github.com/raspberrypi/tools ~/tools
	sudo dpkg --add-architecture armhf
	sudo apt-get update
	sudo apt-get install libirrlicht-dev:armhf cmake:armhf libbz2-dev:armhf libpng12-dev:armhf \
		libjpeg-dev:armhf libxxf86vm-dev:armhf libgl1-mesa-dev:armhf libsqlite3-dev:armhf \
		libhiredis-dev:armhf libogg-dev:armhf libgmp-dev:armhf libvorbis-dev:armhf libopenal-dev:armhf \
		gettext:armhf libpq-dev:armhf libleveldb-dev:armhf
}

# Mac OSX build only
install_macosx_deps() {
	brew update
	brew install freetype gettext hiredis irrlicht leveldb libogg libvorbis luajit
	if brew ls | grep -q jpeg; then
		brew upgrade jpeg
	else
		brew install jpeg
	fi
	#brew upgrade postgresql
}

# Relative to git-repository root:
TRIGGER_COMPILE_PATHS="src/.*\.(c|cpp|h)|CMakeLists.txt|src/CMakeLists.txt|cmake/Modules/|util/travis/|util/buildbot/|.travis.yml"

needs_compile() {
	RANGE="$TRAVIS_COMMIT_RANGE"
	if [[ "$(git diff --name-only $RANGE -- 2>/dev/null)" == "" ]]; then
		RANGE="$TRAVIS_COMMIT^...$TRAVIS_COMMIT"
		echo "Fixed range: $RANGE"
	fi
	git diff --name-only $RANGE -- | egrep -q "^($TRIGGER_COMPILE_PATHS)"
}

