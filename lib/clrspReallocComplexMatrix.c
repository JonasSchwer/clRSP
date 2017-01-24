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

    if (A->layout == CLRSP_PLANAR) {
        A->real = (float*)realloc(A->real, rows * cols * sizeof(float));
        assert(A->real);
        A->imag = (float*)realloc(A->imag, rows * cols * sizeof(float));
        assert(A->imag);
    } else {
        A->real = (float*)realloc(A->real, rows * cols * 2 * sizeof(float));
        assert(A->real);
    }
    A->rows = rows;
    A->cols = cols;
}
