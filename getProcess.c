#include <dirent.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

int digits_only(const char *s)
{
    while (*s)
    {
        if (isdigit(*s++) == 0)
            return 0;
    }
    return 1;
}

typedef struct
{
    pid_t pid;
    char name[256];
    unsigned long long int mem;
} processes;

processes readData(char *dir)
{
    processes p;
    // printf("%s\n", "in here");
    char fileName[4096] = "/proc/";
    strcat(fileName, dir);
    char *statFile = "/stat";
    strcat(fileName, statFile);
    // printf("%s\n", fileName);
    int fd = open(fileName, O_RDONLY);
    // printf("%s\n", "file opened");
    char text[4096];
    int n = read(fd, text, 4096);
    text[n] = '\0';
    // printf("%s\n", text);
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
            // printf("Mem %d\n", mem);
            mem += atoll(token);
            // printf("Mem %d\n", mem);
            p.mem = mem;
            break;
        }
        // printf("%d %s\n", i, token);
        token = strtok(NULL, " ");
        i++;
    }
    close(fd);
    return p;
}

int comparator(const void *p, const void *q)
{
    processes *a = (processes *)p;
    processes *b = (processes *)q;
    return (a->mem) < (b->mem);
    // return ((struct processes *)p)->mem > (struct processes *)q)->mem;
}

int main()
{
    DIR *d;
    struct dirent *dir;
    int c = 0;
    d = opendir("/proc/");
    int size = 0;
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
    d = opendir("/proc/");
    int i = 0;
    processes *arr = (processes *)malloc(sizeof(processes) * size);
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
            if (digits_only(dir->d_name) == 1)
            {
                // printf("%s\n", dir->d_name);
                arr[i] = readData(dir->d_name);
                // arr[i].pid = p.pid;
                // strcpy(arr[i].name, p.name);
                // arr[i].mem = p.mem;
                i++;
                // printf("%d %s %lld", p.pid, p.name, p.mem);
                // break;
                c++;
            }
        }
        closedir(d);
    }
    printf("\n%d", c);
    qsort(arr, size, sizeof(processes), comparator);
    for (int j = 0; j < size; j++)
    {
        printf("\n%d %s %lld", arr[j].pid, arr[j].name, arr[j].mem);
    }
    return (0);
}