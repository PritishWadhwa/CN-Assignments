default:
	mkdir recieved_from_server
	gcc server.c -lpthread -o server
	gcc client.c -o client

clean:
	rm client server
	rm *.txt
	rm recieved_from_server/*.txt