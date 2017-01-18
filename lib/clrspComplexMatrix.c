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
                      size_t cols)
{
    clrspComplexMatrix *A;
    A = (clrspComplexMatrix*)malloc(sizeof(clrspComplexMatrix));
    assert(A);
    A->rows = rows;
    A->cols = cols;
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
