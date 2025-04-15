.PHONY: \
	all \
	debug \
	clean-debug \
	relwithdebinfo \
	clean-relwithdebinfo \
	release \
	clean-release \
	clean

GEN:="Unix Makefiles"
NINJA_PATH:=$(shell command -v ninja 2> /dev/null)
ifdef NINJA_PATH
	GEN:="Ninja"
endif

build-debug:
	cmake -B $@ -DCMAKE_BUILD_TYPE=Debug -G $(GEN) -DCMAKE_CXX_FLAGS_DEBUG="-ggdb3"

debug: build-debug
	cmake --build build-$@

clean-debug:
	rm -rdf build-debug

build-relwithdebinfo:
	cmake -B $@ -DCMAKE_BUILD_TYPE=RelWithDebInfo -G $(GEN) -DCMAKE_CXX_FLAGS_DEBUG="-ggdb1"

relwithdebinfo: build-relwithdebinfo
	cmake --build build-$@

clean-relwithdebinfo:
	rm -rdf build-relwithdebinfo

build:
	cmake -B $@ -DCMAKE_BUILD_TYPE=Release -G $(GEN) -DCMAKE_EXPORT_COMPILE_COMMANDS=ON

release: build
	cmake --build build

clean-release:
	rm -rdf build

all: debug relwithdebinfo release
	@:

clean: clean-debug clean-relwithdebinfo clean-release
	@:
