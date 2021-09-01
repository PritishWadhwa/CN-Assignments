// #include <stdio.h>
// #include <dirent.h>
// #include <sys/stat.h>
// #include <sys/types.h>

// int main()
// {
//     struct dirent *dp;
//     char *dir = "/home/kali/Desktop/CN/Assignment1";
//     DIR *dfd;

//     char filename_qfd[100];
//     char new_name_qfd[100];

//     while ((dp = readdir(dfd)) != NULL)
//     {
//         struct stat stbuf;
//         sprintf(filename_qfd, "%s/%s", dir, dp->d_name);
//         if (stat(filename_qfd, &stbuf) == -1)
//         {
//             printf("Unable to stat file: %s\n", filename_qfd);
//             continue;
//         }

//         if ((stbuf.st_mode & S_IFMT) == S_IFDIR)
//         {
//             continue;
//             // Skip directories
//         }
//         else
//         {
//             char *new_name = get_new_name(dp->d_name); // returns the new string
//                                                        // after removing reqd part
//             sprintf(new_name_qfd, "%s/%s", dir, new_name);
//             rename(filename_qfd, new_name_qfd);
//         }
//     }
// }

#include <dirent.h>
#include <stdio.h>
#include <ctype.h>

int digits_only(const char *s)
{
    while (*s)
    {
        if (isdigit(*s++) == 0)
            return 0;
    }
    return 1;
}

int main()
{
    DIR *d;
    struct dirent *dir;
    int c = 0;
    d = opendir("/proc/");
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
            if (digits_only(dir->d_name) == 1)
            {
                printf("%s\n", dir->d_name);
                c++;
            }
        }
        closedir(d);
    }
    printf("%d", c);
    return (0);
}