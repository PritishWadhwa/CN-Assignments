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

int main()
{
    DIR *d;
    struct dirent *dir;
    d = opendir("./");
    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
            printf("%s\n", dir->d_name);
        }
        closedir(d);
    }
    return (0);
}