all: build-rpc build-dnode build-hub

build-rpc:
	g++ src/rpc.cpp `find src/util src/dnode src/hub -name "*.cpp"` -o bin/rpc

build-dnode:
	g++ src/dnode.cpp `find src/util src/dnode src/hub -name "*.cpp"` -o bin/dnode

build-hub:
	g++ src/hub.cpp `find src/util src/dnode src/hub -name "*.cpp"` -o bin/hub
