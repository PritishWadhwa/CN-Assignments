#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

ssize_t total = 0;
#define MAX_LINE 4096
#define LINSTENPORT 7788
#define SERVERPORT 8877
#define BUFFSIZE 4096

void sendfile(FILE *fp, int sockfd)
{
	int n;
	char sendline[MAX_LINE] = {0};
	while ((n = fread(sendline, sizeof(char), MAX_LINE, fp)) > 0)
	{
		total += n;
		if (n != MAX_LINE && ferror(fp))
		{
			perror("Read File Error");
			exit(1);
		}

		if (send(sockfd, sendline, n, 0) == -1)
		{
			perror("Can't send file");
			exit(1);
		}
		memset(sendline, 0, MAX_LINE);
	}
}

int main(int argc, char **argv)
{
	int port;
	int sock = -1;
	struct sockaddr_in address;
	struct hostent *host;
	int len;
	/* checking commandline parameter */
	if (argc != 4)
	{
		printf("usage: %s hostname port text\n", argv[0]);
		return -1;
	}
	/* obtain port number */
	if (sscanf(argv[2], "%d", &port) <= 0)
	{
		fprintf(stderr, "%s: error: wrong parameter: port\n", argv[0]);
		return -2;
	}
	/* create socket */
	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock <= 0)
	{
		fprintf(stderr, "%s: error: cannot create socket\n", argv[0]);
		return -3;
	}
	/* connect to server */
	address.sin_family = AF_INET;
	address.sin_port = htons(port);
	host = gethostbyname(argv[1]);
	if (!host)
	{
		fprintf(stderr, "%s: error: unknown host %s\n", argv[0], argv[1]);
		return -4;
	}
	memcpy(&address.sin_addr, host->h_addr_list[0], host->h_length);
	if (connect(sock, (struct sockaddr *)&address, sizeof(address)))
	{
		fprintf(stderr, "%s: error: cannot connect to host %s\n", argv[0], argv[1]);
		return -5;
	}
	// /* send text to server */
	// len = strlen(argv[3]);
	// write(sock, &len, sizeof(int));
	// write(sock, argv[3], len);

	FILE *fp = fopen("./send.txt", "rb");
	if (fp == NULL)
	{
		perror("Can't open file");
		exit(1);
	}

	sendfile(fp, sock);
	//puts("Send Success");
	printf("Send Success, NumBytes = %ld\n", total);
	fclose(fp);

	// Close Socket
	close(sock);
	return 0;
}