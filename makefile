all:server client

server: server.cc message.h
	clang++ server.cc -g -std=c++11 -o ../excecutable/server

client: client.cc message.h
	clang++ client.cc -g -std=c++11 -o ../excecutable/client

clean: 
	rm -rf server client
