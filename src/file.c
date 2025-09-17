#include <stdio.h>  // fprintf, stdout, stderr.
#include <stdlib.h> // exit, EXIT_FAILURE, EXIT_SUCCESS.
#include <string.h> // strerror.
#include <errno.h>  // errno.

int print_error(char *path, int errnum)
{
    return fprintf(stdout, "%s Somethings wrong (%s)\n", path, strerror(errnum));
}

int main(int argc, char *argv[])
{
    FILE *f = fopen(argv[1], "r");
    unsigned char c;
    while (fread(&c, sizeof(char), 1, f) == 1)
    {
        printf("%3d ", (int)c);
        if (c > 31 && c < 127)
        {
            fwrite(&c, sizeof(char), 1, stdout);
        }
        printf("\n");
    }
}