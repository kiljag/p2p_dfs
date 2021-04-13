all: build-rpc build-dnode build-hub

build-rpc:
	g++ src/rpc.cpp `find src/common -name "*.cpp"` -o bin/rpc

build-dnode:
	g++ src/dnode.cpp `find src/common -name "*.cpp"` -o bin/dnode

build-hub:
	g++ src/hub.cpp `find src/common -name "*.cpp"` -o bin/hub
