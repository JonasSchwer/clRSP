/******************************************************************************
*
*   Implementation of the clrspComplexMatrix datatype.
*
*   Author(s): Michael Thoma
*
******************************************************************************/

#include "../clRSP.h"

clrspComplexMatrix*
clrspNewComplexMatrix(size_t rows,
                      size_t cols,
                      clrspStorageOrder order)
{
    clrspComplexMatrix *A;
    A = (clrspComplexMatrix*)malloc(sizeof(clrspComplexMatrix));
    assert(A);
    A->rows = rows;
    A->cols = cols;
    A->order = order;
    A->real = NULL;
    A->imag = NULL;

    return A;
}


void
clrspAllocComplexMatrix(clrspComplexMatrix *A)
{
    assert(A);

    free(A->real);
    free(A->imag);
    A->real = (float*)malloc(A->rows * A->cols * sizeof(float));
    assert(A->real);
    A->imag = (float*)malloc(A->rows * A->cols * sizeof(float));
    assert(A->imag);
}


void
clrspReallocComplexMatrix(clrspComplexMatrix *A,
                         size_t rows,
                         size_t cols)
{
    assert(A);

    if (rows != A->rows || cols != A->cols) {
        A->real = (float*)realloc(A->real, rows * cols * sizeof(float));
        assert(A->real);
        A->imag = (float*)realloc(A->imag, rows * cols * sizeof(float));
        assert(A->imag);
        A->rows = rows;
        A->cols = cols;
    }
}


void
clrspFreeComplexMatrix(clrspComplexMatrix *A)
{
    free(A->real);
    free(A->imag);
    free(A);
}


/*
float*
clrspLoadMatrix(size_t rows,
                size_t cols,
                clrspStorageOrder order,
                const char *path)
{
    FILE *fptr;
    if (!(fptr = fopen(path, "r"))) {
        char msg[] = "Error, unable to open";
        char err_msg[strlen(msg) + strlen(path) + 2];
        snprintf(err_msg, strlen(msg) + strlen(path) + 2, "%s %s", msg, path);
        perror((const char*)err_msg);
        return NULL;
    }

    size_t n = 0, i = 0, j = 0, inc_row, inc_col;
    if (order == CLRSP_ROW_MAJOR) {
        inc_row = cols;
        inc_col = 1;
    } else {
        inc_row = 1;
        inc_col = rows;
    }
    char *line;
    char *line_;
    char *end_ptr;
    float *data = (float*)malloc(rows * cols * sizeof(float));
    if (!data) {
        perror("Error, loadMatrix");
        exit(1);
    }
    for (i = 0; i < rows; ++i) {
        getline(&line_, &n, fptr);
        line = line_;
        for (j = 0; j < cols; ++j) {
            data[i * inc_row + j * inc_col] = strtof(line, &end_ptr);
            line = end_ptr;
        }
    }
    fclose(fptr);
    free(line_);

    return data;
}

clrspComplexMatrix*
clrspLoadComplexMatrix(size_t rows,
                       size_t cols,
                       clrspStorageOrder order,
                       const char *re_path,
                       const char *im_path)
{
    clrspComplexMatrix* A = clrspNewComplexMatrix(rows,
                                                  cols,
                                                  order,
                                                  CLRSP_COMPLEX_PLANAR);
    A->real = clrspLoadMatrix(rows, cols, order, re_path);
    A->imag = clrspLoadMatrix(rows, cols, order, im_path);

    return A;
}


int
clrspWriteMatrix(size_t rows,
                 size_t cols,
                 clrspStorageOrder order,
                 const float* data,
                 const char *path)
{
    FILE *fptr;
    if (!(fptr = fopen(path, "w"))) {
        char msg[] = "Error, unable to open for write";
        char err_msg[strlen(msg) + strlen(path) + 2];
        snprintf(err_msg, strlen(msg) + strlen(path) + 2, "%s %s", msg, path);
        perror((const char*)err_msg);
        return -1;
    }

    size_t i, j, inc_row, inc_col;
    if (order == CLRSP_ROW_MAJOR) {
        inc_row = cols;
        inc_col = 1;
    } else {
        inc_row = 1;
        inc_col = rows;
    }
    for (i = 0; i < rows; ++i) {
        for (j = 0; j < cols; ++j) {
            fprintf(fptr, "%g ", data[i * inc_row + j * inc_col]);
        }
        fprintf(fptr, "\n");
    }
    fclose(fptr);

    return 0;
}

void
clrspWriteComplexMatrix(clrspComplexMatrix *A,
                        const char *re_path,
                        const char *im_path)
{
    clrspWriteMatrix(A->rows, A->cols, A->order, A->real, re_path);
    clrspWriteMatrix(A->rows, A->cols, A->order, A->imag, im_path);
}


void
clrspPrintComplexMatrix(const clrspComplexMatrix *A)
{
    assert(A);

    size_t i, j, inc_row, inc_col;
    if (A->order == CLRSP_ROW_MAJOR) {
        inc_row = A->cols;
        inc_col = 1;
    } else {
        inc_row = 1;
        inc_col = A->rows;
    }
    for (i = 0; i < A->rows; ++i) {
        for (j = 0; j < A->cols; ++j) {
            printf("%+2f%+2fi ",
                   A->real[i * inc_row + j * inc_col],
                   A->imag[i * inc_row + j * inc_col]);
        }
        printf("\n");
    }
    printf("\n");
}
*/
