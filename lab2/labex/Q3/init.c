#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char **argv)
{

    /**
     * TODO: Write your code here.
     */

    if (argc != 3)
    {
        printf("Error\n");
        exit(1);
    }
    int fd = open(argv[2], O_RDONLY);
    if (fd < 0)
    {
        printf("Error\n");
        exit(1);
    }

    unsigned long long size = lseek(fd, 0, SEEK_END);
    if (size < 0)
    {
        printf("Error\n");
        close(fd);
        return 1;
    }

    lseek(fd, 0, SEEK_SET);

    // char buff[size];
    char *buff = (char *)malloc(size + 1);
    if (!buff)
    {
        printf("Error\n");
        close(fd);
        return 1;
    }
    unsigned long long bytes = read(fd, buff, size);

    if (bytes < 0)
    {
        printf("Error\n");
        free(buff);
        close(fd);
        return 1;
    }
    buff[bytes] = '\0';
    // int status =

    if (strstr(buff, argv[1]) != NULL)
        printf("FOUND\n");
    else
        printf("NOT FOUND\n");
    free(buff);
    close(fd);
    return 0;
}
