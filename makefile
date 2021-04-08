all:server client

server: server.cc message.h
	g++ server.cc -g -o /Users/keekeeliu/Downloads/lab2_2021/excecutable/server

client: client.cc message.h
	g++ client.cc -g -o /Users/keekeeliu/Downloads/lab2_2021/excecutable/client

clean: 
	rm -rf server client
