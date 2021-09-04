default:
	mkdir -p recieved_from_server
	gcc server.c -lpthread -o server
	gcc client.c -o client

clean:
	rm client server
	rm *.txt
	rm recieved_from_server/*.txt
	rm -r recieved_from_server

client:
	mkdir -p recieved_from_server
	gcc client.c -o client

server:
	gcc server.c -lpthread -o server

cleanClient:
	rm client
	rm recieved_from_server/*.txt
	rm -r recieved_from_server

cleanServer:
	rm server
	rm *.txt

