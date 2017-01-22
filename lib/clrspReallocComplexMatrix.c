/******************************************************************************
*
*   Implementation of the clrspComplexMatrix datatype.
*
*   Author(s): Michael Thoma
*
******************************************************************************/

#include "../clRSP.h"

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
