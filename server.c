#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/socket.h>
// #include <linux/in.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>

#include <arpa/inet.h>

#include <sys/socket.h>

#define MAX_LINE 4096
#define LINSTENPORT 7788
#define SERVERPORT 8877
#define BUFFSIZE 4096

ssize_t total = 0;

typedef struct
{
    int sock;
    struct sockaddr address;
    int addr_len;
} connection_t;

void writefile(int sockfd, FILE *fp)
{
    ssize_t n;
    char buff[MAX_LINE] = {0};
    while ((n = recv(sockfd, buff, MAX_LINE, 0)) > 0)
    {
        total += n;
        if (n == -1)
        {
            perror("Receive File Error");
            exit(1);
        }

        if (fwrite(buff, sizeof(char), n, fp) != n)
        {
            perror("Write File Error");
            exit(1);
        }
        memset(buff, 0, MAX_LINE);
    }
}

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

void *process(void *ptr)
{
    char *buffer;
    int len;
    connection_t *conn;
    long addr = 0;

    if (!ptr)
        pthread_exit(0);
    conn = (connection_t *)ptr;

    // Part added
    char filename[BUFFSIZE] = {0};
    if (recv(conn->sock, filename, BUFFSIZE, 0) == -1)
    {
        perror("Can't receive filename");
        exit(1);
    }

    FILE *fp = fopen(filename, "wb");
    if (fp == NULL)
    {
        perror("Can't open file");
        exit(1);
    }

    char address[INET_ADDRSTRLEN];
    printf("Start receive file: %s from %s\n", filename,
           inet_ntop(AF_INET,
                     ((struct sockaddr_in *)&conn->address)->sin_addr.s_addr,
                     address, INET_ADDRSTRLEN));
    writefile(conn->sock, fp);
    printf("Receive Success, NumBytes = %ld\n", total);

    // /* read length of message */
    // read(conn->sock, &len, sizeof(int));
    // if (len > 0)
    // {
    //     addr = (long)((struct sockaddr_in *)&conn->address)->sin_addr.s_addr;
    //     buffer = (char *)malloc((len + 1) * sizeof(char));
    //     buffer[len] = 0;

    //     /* read message */
    //     read(conn->sock, buffer, len);

    //     /* print message */
    //     printf("%d.%d.%d.%d: %s\n",
    //            (int)((addr)&0xff),
    //            (int)((addr >> 8) & 0xff),
    //            (int)((addr >> 16) & 0xff),
    //            (int)((addr >> 24) & 0xff),
    //            buffer);
    //     free(buffer);
    // }

    /* close socket and clean up */
    close(conn->sock);
    free(conn);
    pthread_exit(0);
}

int main(int argc, char **argv)
{
    int sock = -1;
    struct sockaddr_in address;
    // Temporary Port added
    int port = 8877;
    connection_t *connection;
    pthread_t thread;

    /* check for command line arguments */
    if (argc != 2)
    {
        fprintf(stderr, "usage: %s port\n", argv[0]);
        return -1;
    }

    /* obtain port number */
    if (sscanf(argv[1], "%d", &port) <= 0)
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

    /* bind socket to port */
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    if (bind(sock, (struct sockaddr *)&address, sizeof(struct sockaddr_in)) < 0)
    {
        fprintf(stderr, "%s: error: cannot bind socket to port %d\n", argv[0], port);
        return -4;
    }

    /* listen on port */
    if (listen(sock, 5) < 0)
    {
        fprintf(stderr, "%s: error: cannot listen on port\n", argv[0]);
        return -5;
    }

    printf("%s: ready and listening\n", argv[0]);

    while (1)
    {
        /* accept incoming connections */
        connection = (connection_t *)malloc(sizeof(connection_t));
        connection->sock = accept(sock, &connection->address, &connection->addr_len);
        if (connection->sock <= 0)
        {
            free(connection);
        }
        else
        {
            /* start a new thread but do not wait for it */
            pthread_create(&thread, 0, process, (void *)connection);
            pthread_detach(thread);
        }
    }

    return 0;
}