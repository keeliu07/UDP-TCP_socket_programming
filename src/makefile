all:server client

server: server.cc message.h
	clang++ server.cc -g -std=c++11 -o ../executable/server

client: client.cc message.h
	clang++ client.cc -g -std=c++11 -o ../executable/client

clean: 
	rm -rf server client
