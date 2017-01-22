/******************************************************************************
*
*   Implementation of auxiliary.
*
*   Author(s): Michael Thoma
*
******************************************************************************/

#include "../clRSP.h"

char*
clrspLoadKernelSource(const char *path)
{
    FILE *fptr;
    if (!(fptr = fopen(path, "r"))) {
        char msg[] = "Error, unable to open";
        char err_msg[strlen(msg) + strlen(path) + 2];
        snprintf(err_msg, strlen(msg)+strlen(path)+2, "%s %s", msg, path);
        perror((const char*)err_msg);
        return NULL;
    }

    size_t n;
    size_t n_total = 0;
    int resizes = 1;
    char read_buffer[128];
    char *kernel_source = (char*)malloc(128 * 10 * resizes * sizeof(char));
    while (!feof(fptr)) {
        n = fread((void*)read_buffer, sizeof(char), 128, fptr);
        n_total += n;
        if (n_total >= 128 * 10 * resizes) {
            kernel_source = (char*)realloc(kernel_source,
                                           128 * 10 * (++resizes)
                                           * sizeof(char));
        }
        memcpy(&kernel_source[n_total-n], (const char*)read_buffer, n);
    }
    fclose(fptr);
    kernel_source[n_total] = '\0';
    kernel_source = (char*)realloc(kernel_source, n_total * sizeof(char));

    return kernel_source;
}
