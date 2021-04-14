all: rpc dnode hub

rpc:
	g++ src/rpc.cpp `find src/util src/dnode src/hub -name "*.cpp"` -o bin/rpc

dnode:
	g++ src/dnode.cpp `find src/util src/dnode src/hub -name "*.cpp"` -o bin/dnode

hub:
	g++ src/hub.cpp `find src/util src/dnode src/hub -name "*.cpp"` -o bin/hub
