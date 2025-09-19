#include <stdio.h>  // fprintf, stdout, stderr.
#include <stdlib.h> // exit, EXIT_FAILURE, EXIT_SUCCESS.
#include <string.h> // strerror.
#include <errno.h>  // errno.
#include <assert.h>

int print_error(char *path, int errnum) {
    return fprintf(stdout, "%s: cannot determine (%s)\n", path, strerror(errnum));
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: file path\n");
        return EXIT_FAILURE;
    }

    FILE *f = fopen(argv[1], "r");
    if (!f) {  // Testing for non-existing file
        print_error(argv[1], errno);
        return EXIT_SUCCESS;
    }

    unsigned char c;

    // Testing for empty; differenting between error and end-of-file
    size_t r = fread(&c, sizeof(char), 1, f);
    if (r == 0) { 
        if (ferror(f)) {
            int e = errno; fclose(f);
            print_error(argv[1],e);
            return EXIT_SUCCESS;
        }
        printf("%s: empty\n", argv[1]);
        fclose(f);
        return EXIT_SUCCESS;
    }

    // Testing for ascii
    fseek(f, 0, SEEK_SET);

    if (fseek(f, 0, SEEK_SET) != 0) { // error check
        int e = errno; 
        fclose(f);
        print_error(argv[1], e);
    }
    int isAscii = 1;
    while ((r = fread(&c, sizeof(char), 1, f) == 1)) {
        if (c < 7 || (c > 13 && c < 27) || (c > 27 && c < 32) || c > 127) {
            isAscii = 0;
            break;
        }
    }
    if (r == 0 && ferror(f)) {
        int e = errno;
        fclose(f);
        print_error(argv[1], e);
        return EXIT_SUCCESS;
    }
    if (isAscii == 1) {
        printf("%s: ASCII text\n", argv[1]);
        fclose(f);
        return EXIT_SUCCESS;
    }

    // Testing for ISO
    fseek(f, 0, SEEK_SET);

    if (fseek(f, 0, SEEK_SET) != 0) { // error check
        int e = errno;
        fclose(f);
        print_error(argv[1], e);
        return EXIT_SUCCESS;
    }
    int isIso = 1;
    while ((r = fread(&c, sizeof(char), 1, f)) == 1) {
        if (c < 7 || (c > 13 && c < 27) || (c > 27 && c < 32) || (c > 127 && c < 160)) {
            isIso = 0;
            break;
        }
    }
    if (r == 0 && ferror(f)) {
        int e = errno;
        fclose(f);
        print_error(argv[1], e);
        return EXIT_SUCCESS;
    }
    if (isIso == 1) {
        printf("%s: ISO-8859 text\n", argv[1]);
        return EXIT_SUCCESS;
    }

    // Testing for UTF
    fseek(f, 0, SEEK_SET);

    if (fseek(f, 0 ,SEEK_SET) != 0) { // error check
        int e = errno;
        fclose(f);
        print_error(argv[1], e);
        return EXIT_SUCCESS;
    }

    int isUtf = 1;
    while ((r = fread(&c, sizeof(char), 1, f)) == 1) {
        if (c == 0 || c > 247 || (c > 127 && c < 192)) {
            isUtf = 0;
            break;
        } else if (c > 191 && c < 224) {
            if (fread(&c, sizeof(char), 1, f) != 1 || c < 128 || c > 191) {
                isUtf = 0;
                break;
            }
        } else if (c > 223 && c < 240) {
            if (fread(&c, sizeof(char), 1, f) != 1 || c < 128 || c > 191) {
                isUtf = 0;
                break;
            }
            if (fread(&c, sizeof(char), 1, f) != 1 || c < 128 || c > 191) {
                isUtf = 0;
                break;
            }
        } else if (c > 239 && c < 248) {
            if (fread(&c, sizeof(char), 1, f) != 1 || c < 128 || c > 191) {
                isUtf = 0;
                break;
            }
            if (fread(&c, sizeof(char), 1, f) != 1 || c < 128 || c > 191) {
                isUtf = 0;
                break;
            }
            if (fread(&c, sizeof(char), 1, f) != 1 || c < 128 || c > 191) {
                isUtf = 0;
                break;
            }
        }
    }

    if (r == 0 && ferror(f)) {
        int e = errno;
        fclose(f);
        print_error(argv[1], e);
        return EXIT_SUCCESS;
    }

    if (isUtf == 1) {
        printf("%s: UTF-8 Unicode text\n", argv[1]);
        return EXIT_SUCCESS;
    }
    printf("%s: data\n", argv[1]);
    fclose(f);
    return EXIT_SUCCESS;
}