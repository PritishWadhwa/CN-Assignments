#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main()
{
    time_t start = time(NULL);
    while (time(NULL) - start < 121)
    {
        system("./client 8080 localhost 10");
    }
    time_t end = time(NULL);
    printf("%.2f", (double)(end - start));
    return 0;
}