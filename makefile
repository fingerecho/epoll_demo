
server:
	g++ main.cpp epollServer.cpp  -lpthread -o server

client: 
	g++ mainClient.cpp epollClient.cpp -lpthread -o client	