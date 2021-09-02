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

typedef struct
{
    pid_t pid;
    char name[256];
    unsigned long long int mem;
} processes;

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

    // Receive the result (file with top n processes)
    // get the filename
    char tempfilename[4096] = {0};
    if (recv(sockId, tempfilename, sizeof(tempfilename), 0) <= 0)
    {
        perror("Error: File name not received");
        exit(1);
    }
    printf("%s\n", tempfilename);
    char topNFileName[4200] = "./recieved_from_server/";
    strcat(topNFileName, tempfilename);
    FILE *fp = fopen(topNFileName, "wb");
    if (fp == NULL)
    {
        perror("Error: File not opened");
        exit(1);
    }

    // get the file data
    int temp;
    char buffer[4096] = {0};
    while ((temp = recv(sockId, buffer, sizeof(buffer), 0)) > 0)
    {
        if (n == -1)
        {
            perror("Error: File not received");
            exit(1);
        }
        if (fwrite(buffer, sizeof(char), temp, fp) != temp)
        {
            perror("Error: File Write Error");
            exit(1);
        }
        memset(buffer, 0, sizeof(buffer));
    }
    fclose(fp);

    // get the top functioning process
    processes topProcess;
    unsigned long long int topMem = -1;
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
                if (temp.mem > topMem)
                {
                    topMem = temp.mem;
                    topProcess = temp;
                }
            }
        }
        closedir(d);
    }

    // saving the top process in a file
    FILE *interfp;

    char topFileName[4096] = "top_process.txt";
    interfp = fopen(topFileName, "w");
    fprintf(interfp, "%s %d %llu\n", topProcess.name, topProcess.pid, topProcess.mem);
    fclose(interfp);

    // sending the file with the top process to the server
    char finalBuffer[4096] = {0};
    // Sending the filename
    strncpy(finalBuffer, topFileName, strlen(topFileName));
    if (send(sockId, finalBuffer, strlen(finalBuffer), 0) < 0)
    {
        perror("error in sending filename");
        exit(1);
    }
    sleep(1);
    // sending contents of the file
    FILE *filePtr = fopen(topFileName, "rb");
    char sendLines[4096] = {0};
    while ((temp = fread(sendLines, sizeof(char), 4096, filePtr)) > 0)
    {
        if (temp != 4096 && ferror(filePtr))
        {
            perror("error in reading file");
            exit(1);
        }
        if (send(sockId, sendLines, temp, 0) < 0)
        {
            perror("error in sending file");
            exit(1);
        }
        memset(sendLines, 0, 4096);
    }
    fclose(filePtr);
    close(sockId);
    return 0;
}
