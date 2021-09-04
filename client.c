#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <dirent.h>
#include <ctype.h>
#include <fcntl.h>

// structure to define a process
typedef struct
{
    pid_t pid;
    char name[256];
    unsigned long long int mem;
} processes;

// function to find out whether a directory corresponds to pid or not
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

// function to get the process name, its pid and the memory usage
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

// command line parameters: port, host, n
int main(int argc, char *argv[])
{
    int port;
    struct hostent *host;
    int n;

    struct sockaddr_in clientAddr;

    if (argc != 4)
    {
        fprintf(stderr, "Parameters missing\n");
        exit(1);
    }

    // Get the port number
    if (sscanf(argv[1], "%d", &port) <= 0)
    {
        fprintf(stderr, "Invalid port number\n");
        exit(1);
    }

    // Get the host name
    host = gethostbyname(argv[2]);
    if (!host)
    {
        fprintf(stderr, "Invalid host name\n");
        exit(1);
    }

    // Get the value of n
    n = atoi(argv[3]);

    // Create a socket
    int sockId = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sockId <= 0)
    {
        fprintf(stderr, "Error creating socket\n");
        exit(1);
    }

    // Connect to the server
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_port = htons(port);
    memcpy(&clientAddr.sin_addr, host->h_addr_list[0], host->h_length);
    if (connect(sockId, (struct sockaddr *)&clientAddr, sizeof(clientAddr)))
    {
        fprintf(stderr, "Error connecting to server\n");
        exit(1);
    }

    // Send the value of n
    int len = strlen(argv[3]);
    write(sockId, &len, sizeof(int));
    write(sockId, argv[3], len);

    // Receive the result (file with top n processes)
    // get the filename
    char tempfilename[4096] = {0};
    if (recv(sockId, tempfilename, sizeof(tempfilename), 0) <= 0)
    {
        fprintf(stderr, "Error receiving filename\n");
        exit(1);
    }

    // printf("%s\n", tempfilename);
    char topNFileName[4200] = "./recieved_from_server/";
    strcat(topNFileName, tempfilename);
    FILE *fp = fopen(topNFileName, "wb");
    if (fp == NULL)
    {
        fprintf(stderr, "Error opening file\n");
        exit(1);
    }

    // get the file data
    long dataSize;
    long address;
    char *buff1;
    read(sockId, &dataSize, sizeof(long));
    printf("%ld\n", dataSize);
    if (dataSize > 0)
    {
        buff1 = (char *)malloc((dataSize + 1) * sizeof(char));
        buff1[dataSize] = 0;

        // read the text
        read(sockId, buff1, dataSize);
    }
    printf("File: %s recieved from the server\n", tempfilename);
    fprintf(fp, "%s", buff1);
    printf("File: %s saved in the directory: recieved_from_server\n", tempfilename);
    fclose(fp);

    sleep(1);

    // get the top functioning process
    processes topProcess;
    unsigned long long int topMem = 0;
    int flag = 0;
    DIR *d;
    struct dirent *dir;
    d = opendir("/proc/");

    // getting data of each process and storing the one with max memory
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
            if (digits_only(dir->d_name) == 1)
            {
                processes temp = readData(dir->d_name);
                if (flag == 0)
                {
                    topProcess = temp;
                    topMem = temp.mem;
                    flag = 1;
                }
                if (temp.mem > topMem)
                {
                    topMem = temp.mem;
                    topProcess = temp;
                }
            }
        }
        closedir(d);
    }

    // sending the top process to the server
    char buff[4096] = {0};
    sprintf(buff, "%s %d %llu\n", topProcess.name, topProcess.pid, topProcess.mem);

    len = strlen(buff);

    // sending the length of the data
    write(sockId, &len, sizeof(int));

    // sending the data of the top cpu consuming process to the server
    write(sockId, buff, len);
    printf("Top CPU consuming process sent to the server\n");
    close(sockId);
    return 0;
}
