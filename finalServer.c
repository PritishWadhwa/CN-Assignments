#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <dirent.h>
#include <ctype.h>
#include <fcntl.h>

typedef struct
{
    pid_t pid;
    char name[256];
    unsigned long long int mem;
} processes;

typedef struct
{
    int sockid;
    struct sockaddr address;
    int addrlen;
} connection;

int comparator(const void *p, const void *q)
{
    processes *a = (processes *)p;
    processes *b = (processes *)q;
    return (a->mem) < (b->mem);
}

int digits_only(const char *s)
{
    while (*s)
    {
        if (isdigit(*s++) == 0)
        {
            return 0;
        }
    }
    return 1;
}

processes readData(char *dir)
{
    processes p;
    char fileName[4096] = "/proc/";
    strcat(fileName, dir);
    char *statFile = "/stat";
    strcat(fileName, statFile);
    int fd = open(fileName, O_RDONLY);
    char text[4096];
    int n = read(fd, text, 4096);
    text[n] = '\0';
    int i = 0;
    unsigned long long int mem = 0;
    char *token = strtok(text, " ");
    while (token != NULL)
    {
        if (i == 0)
        {
            p.pid = atoi(token);
        }
        if (i == 1)
        {
            strcpy(p.name, token);
        }
        if (i == 13)
        {
            mem = atoll(token);
        }
        if (i == 14)
        {
            mem += atoll(token);
            p.mem = mem;
            break;
        }
        token = strtok(NULL, " ");
        i++;
    }
    close(fd);
    return p;
}

void *getAndSendData(void *ptr)
{
    connection *conn;
    int len;
    char *buff;
    long address = 0;
    int n;
    if (!ptr)
    {
        pthread_exit(0);
    }
    conn = (connection *)ptr;

    // Read the value of n from the client
    // get len of text
    read(conn->sockid, &len, sizeof(int));
    if (len > 0)
    {
        address = (long)((struct sockaddr_in *)&conn->address)->sin_addr.s_addr;
        buff = (char *)malloc((len + 1) * sizeof(char));
        buff[len] = 0;

        // read the text
        read(conn->sockid, buff, len);
        n = atoi(buff);

        free(buff);
    }
    printf("%d.%d.%d.%d on Port %d: %d\n",
           (int)((address)&0xff),
           (int)((address >> 8) & 0xff),
           (int)((address >> 16) & 0xff),
           (int)((address >> 24) & 0xff),
           ((struct sockaddr_in *)&conn->address)->sin_port,
           n);

    // Get the top n pid's
    DIR *d;
    struct dirent *dir;
    d = opendir("/proc/");
    int size = 0;
    // get number of active processes
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
            if (digits_only(dir->d_name) == 1)
            {
                size++;
            }
        }
        closedir(d);
    }

    // getting data of each process
    d = opendir("/proc/");
    int i = 0;
    processes *arr = (processes *)malloc(size * sizeof(processes));
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
            if (digits_only(dir->d_name) == 1)
            {
                arr[i] = readData(dir->d_name);
                i++;
            }
        }
        closedir(d);
    }

    // sorting in decreasing cpu time
    qsort(arr, size, sizeof(processes), comparator);

    close(conn->sockid);
    free(conn);
    pthread_exit(0);
}

int main(int argc, char *argv[])
{
    int port;
    pthread_t thread;
    // Obtaining Port Number
    if (argc != 2)
    {
        fprintf(stderr, "Port not provided\n");
        exit(1);
    }
    if (sscanf(argv[1], "%d", &port) <= 0)
    {
        fprintf(stderr, "Invalid port\n");
        exit(1);
    }

    // Creating Socket
    int sockid = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockid <= 0)
    {
        fprintf(stderr, "Socket creation failed\n");
        exit(1);
    }

    // Binding Socket
    struct sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddress.sin_port = htons(port);

    if (bind(sockid, (const struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0)
    {
        fprintf(stderr, "Binding failed\n");
        exit(1);
    }

    // Listening
    if (listen(sockid, 10) < 0)
    {
        fprintf(stderr, "Listening failed\n");
        exit(1);
    }

    printf("Server is listening on port %d and IP %s\n", port, inet_ntoa(serverAddress.sin_addr));

    connection *client;

    while (1)
    {
        // Accepting incoming connections
        client = (connection *)malloc(sizeof(connection));
        client->sockid = accept(sockid, &client->address, &client->addrlen);
        if (client->sockid <= 0)
        {
            free(client);
        }
        else
        {
            pthread_create(&thread, NULL, getAndSendData, (void *)client);
            pthread_detach(thread);
        }
    }

    return 0;
}