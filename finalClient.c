#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>

// command line parameters: port, host, n
int main(int argc, char *argv[])
{
    int port;
    struct hostent *host;
    int n;

    struct sockaddr_in clientAddr;

    if (argc != 4)
    {
        perror("Error: Parameters missing");
        exit(1);
    }

    // Get the port number
    if (sscanf(argv[1], "%d", &port) <= 0)
    {
        perror("Error: Invalid port number");
        exit(1);
    }

    // Get the host name
    host = gethostbyname(argv[2]);
    if (!host)
    {
        perror("Error: Invalid host name");
        exit(1);
    }

    // Get the value of n
    n = atoi(argv[3]);

    // Create a socket
    int sockId = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockId <= 0)
    {
        perror("Error: Socket creation failed");
        exit(1);
    }

    // Connect to the server
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_port = htons(port);
    memcpy(&clientAddr.sin_addr, host->h_addr_list[0], host->h_length);
    if (connect(sockId, (struct sockaddr *)&clientAddr, sizeof(clientAddr)))
    {
        perror("Error: Connection failed");
        exit(1);
    }

    // Send the value of n
    int len = strlen(argv[3]);

    write(sockId, &len, sizeof(int));
    write(sockId, argv[3], len);
    printf("%s", argv[3]);
    return 0;
}
