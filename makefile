all:server client

server: server.cc message.h
	clang++ server.cc -g -std=c++11 -o /Users/keekeeliu/Downloads/lab2_2021/excecutable/server

client: client.cc message.h
	clang++ client.cc -g -std=c++11 -o /Users/keekeeliu/Downloads/lab2_2021/excecutable/client

clean: 
	rm -rf server client
