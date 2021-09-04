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

pthread_mutex_t lock;
int fileCtr = 0;
// char *itoa(int, char *, int);

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

    // creating a file with the top n processes
    FILE *fp;

    char topNFileName[4096] = "top_";
    char underscore[2] = "_";
    char number[10];
    char currFileCtr[10];
    char extension[5] = ".txt";
    // itoa(n, number, 10);
    // itoa(fileCtr, currFileCtr, 10);
    sprintf(number, "%d", n);
    pthread_mutex_lock(&lock);
    sprintf(currFileCtr, "%d", fileCtr);
    fileCtr++;
    pthread_mutex_unlock(&lock);
    strcat(topNFileName, number);
    strcat(topNFileName, underscore);
    strcat(topNFileName, currFileCtr);
    strcat(topNFileName, extension);
    fp = fopen(topNFileName, "w");
    for (int ct = 0; ct < n; ct++)
    {
        fprintf(fp, "%s %d %llu\n", arr[ct].name, arr[ct].pid, arr[ct].mem);
    }
    fclose(fp);

    // Sending the file to the client
    char buffer[4096] = {0};
    // Sending the filename
    strncpy(buffer, topNFileName, strlen(topNFileName));
    if (send(conn->sockid, buffer, strlen(buffer), 0) < 0)
    {
        perror("error in sending filename");
        exit(1);
    }
    printf("Sending file %s\n", topNFileName);
    // printf("%s\n", buffer);
    sleep(1);
    // sending contents of the file
    printf("Sending file contents\n");
    long fileSize;
    char *fileBuffer;
    FILE *filePtr = fopen(topNFileName, "rb");

    fseek(filePtr, 0L, SEEK_END);
    fileSize = (ftell(filePtr));
    rewind(filePtr);

    fileBuffer = calloc(1, fileSize + 1);

    if (1 != fread(buffer, fileSize, 1, filePtr))
    {
        fclose(filePtr);
        // free(buffer);
        fputs("entire read fails", stderr);
        exit(1);
    }
    printf("%s\n", buffer);
    printf("%ld\n", fileSize);
    // int len = strlen(argv[3]);
    write(conn->sockid, &fileSize, sizeof(long));
    write(conn->sockid, buffer, fileSize);
    fclose(filePtr);
    printf("\n");
    printf("file sent\n");
    printf("File sent\n");
    // int temp;
    // char sendLines[4096] = {0};
    // while ((temp = fread(sendLines, sizeof(char), 4096, filePtr)) > 0)
    // {
    //     // printf("sending\n");
    //     // printf("%s", sendLines);
    //     printf("sending\n");
    //     if (temp != 4096 && ferror(filePtr))
    //     {
    //         perror("error in reading file");
    //         exit(1);
    //     }
    //     // sendLines[strlen(sendLines) - 1] = '\0';
    //     if (send(conn->sockid, sendLines, temp, 0) < 0)
    //     {
    //         perror("error in sending file");
    //         exit(1);
    //     }
    //     // printf("%s", sendLines);
    //     memset(sendLines, 0, 4096);
    //     // printf("%s", sendLines);
    // }
    // fclose(filePtr);
    printf("file sent\n");

    read(conn->sockid, &len, sizeof(int));
    printf("%d\n", len);
    if (len > 0)
    {
        address = (long)((struct sockaddr_in *)&conn->address)->sin_addr.s_addr;
        buff = (char *)malloc((len + 1) * sizeof(char));
        buff[len] = 0;

        // read the text
        read(conn->sockid, buff, len);
        // n = atoi(buff);

        // free(buff);
    }
    printf("%s\n", buff);
    // Read the top process from the client
    // get len of text
    // read(conn->sockid, &len, sizeof(int));
    // if (len > 0)
    // {
    //     address = (long)((struct sockaddr_in *)&conn->address)->sin_addr.s_addr;
    //     buff = (char *)malloc((len + 1) * sizeof(char));
    //     buff[len] = 0;

    //     // read the text
    //     read(conn->sockid, buff, len);
    //     // n = atoi(buff);
    //     printf("%s", buff);
    //     free(buff);
    // }

    // Recieve file with top id from client
    // get the filename
    // sleep(5);

    // char tempfilename[4096] = {0};
    // if (recv(conn->sockid, tempfilename, sizeof(tempfilename), 0) <= 0)
    // {
    //     perror("Error: File name not received");
    //     exit(1);
    // }
    // printf("%s\n", tempfilename);
    // char topFileName[4200] = "./recieved_from_client/";
    // strcat(topFileName, tempfilename);
    // FILE *newfp = fopen(topFileName, "wb");
    // if (newfp == NULL)
    // {
    //     perror("Error: File not opened");
    //     exit(1);
    // }

    // // get the file data
    // char finalbuffer[4096] = {0};
    // while ((temp = recv(conn->sockid, finalbuffer, sizeof(finalbuffer), 0)) > 0)
    // {
    //     if (n == -1)
    //     {
    //         perror("Error: File not received");
    //         exit(1);
    //     }
    //     if (fwrite(finalbuffer, sizeof(char), temp, newfp) != temp)
    //     {
    //         perror("Error: File Write Error");
    //         exit(1);
    //     }
    //     memset(finalbuffer, 0, sizeof(finalbuffer));
    // }
    // fclose(newfp);

    close(conn->sockid);
    free(conn);
    pthread_exit(0);
    printf("\n");
}

int main(int argc, char *argv[])
{
    if (pthread_mutex_init(&lock, NULL) != 0)
    {
        fprintf(stderr, "mutex init failed\n");
        exit(1);
    }

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

    if (bind(sockid, (const struct sockaddr *)&serverAddress, sizeof(serverAddress)) == -1)
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
    pthread_mutex_destroy(&lock);
    return 0;
}