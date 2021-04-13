all: build-rpc build-dnode build-hub

build-rpc:
	g++ rpc.cpp `find util -name "*.cpp"` -o bin/rpc

build-dnode:
	g++ dnode.cpp `find util -name "*.cpp"` -o bin/dnode

build-hub:
	g++ hub.cpp `find util -name "*.cpp"` -o bin/hub
