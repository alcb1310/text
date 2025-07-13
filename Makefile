clean:
	rm -rf ./build

all: clean 
	cmake -S . -B build
	cmake --build build

build:
	cmake --build build
