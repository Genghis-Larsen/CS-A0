#include <stdio.h>  // fprintf, stdout, stderr.
#include <stdlib.h> // exit, EXIT_FAILURE, EXIT_SUCCESS.
#include <string.h> // strerror.
#include <errno.h>  // errno.
#include <assert.h>
#include <stdbool.h>

int print_error(char *path, int errnum) {
    fprintf(stdout, "%s: cannot determine (%s)\n", path, strerror(errnum));
    return EXIT_SUCCESS;
}

int match_found(char *path, char *type, FILE *file) {
    printf("%s: %s\n", path, type);
    int status = fclose(file);
    if (status != 0) {
        return print_error(path, status);
    }
    return EXIT_SUCCESS;
}

bool in_range(unsigned char byte, int inclusive_from, int inclusive_to) {
    return byte >= inclusive_from && byte <= inclusive_to; 
}

int main(int argc, char *argv[]) {

    // Checking arguments
    if (argc != 2) {
        fprintf(stderr, "Usage: file path\n");
        return EXIT_FAILURE;
    }

    // Setup
    char *path = argv[1];
    FILE *f = fopen(path, "r");
    if (!f) {  // Testing for non-existing file
        return print_error(path, errno);
    }
    unsigned char c;

    // Testing for empty; differenting between error and end-of-file
    size_t r = fread(&c, sizeof(char), 1, f);
    if (r == 0) { 
        if (ferror(f)) {
            int e = errno;
            fclose(f);
            return print_error(path, e);
        }
        return match_found(path, "empty", f);
    }

    // Testing for ascii and iso
    if (fseek(f, 0, SEEK_SET) != 0) { // error check
        int e = errno; 
        fclose(f);
        return print_error(path, e);
    }
    bool ascii = true;
    bool iso = true;
    while (fread(&c, sizeof(char), 1, f) == 1) {
        if (byte < 7 || in_range(byte, 14, 27) || in_range(byte, 28, 31) || in_range(byte, 127, 159)) {
            ascii = false;
            iso = false;
            break;
        } else if (byte > 159) {
            ascii = false;
        }
    }
    if (ferror(f) != 0) {
        int e = errno;
        fclose(f);
        return print_error(path, e);
    }
    if (ascii) {
        return match_found(path, "ASCII text", file);
    } else if (iso) {
        return match_found(path, "ISO-8859 text", file);
    }

    // Testing for utf
    if (fseek(f, 0, SEEK_SET) != 0) { // error check
        int e = errno;
        fclose(f);
        return print_error(path, e);
    }
    bool utf = true;
    while (fread(&c, sizeof(char), 1, f) == 1) {
        int next = 0;
        if (byte == 0 || byte > 247 || in_range(byte, 128, 191)) {
            utf = false;
            break;
        } else if (in_range(byte, 192, 223)) {
            next = 1;
        } else if (in_range(byte, 224, 239)) {
            next = 2;
        } else if (in_range(byte, 240, 249)) {
            next = 3;
        }
        while (next-- > 0) {
            if (fread(&byte, sizeof(char), 1, file) == 0 || !in_range(byte, 128, 191)) {
                int status = ferror(file);
                if (status != 0) {
                    return print_error(path, status);
                }
                utf = false;
                break;
            }
        }
    }
    if (ferror(f) != 0) {
        int e = errno;
        fclose(f);
        return print_error(path, e);
    }
    if (utf) {
        return match_found(path, "UTF-8 Unicode text", file);
    }
    return match_found(path, "data", file);
}