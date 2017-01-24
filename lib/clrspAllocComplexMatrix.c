/******************************************************************************
*
*   Implementation of the clrspComplexMatrix datatype.
*
*   Author(s): Michael Thoma
*
******************************************************************************/

#include "../clRSP.h"

void
clrspAllocComplexMatrix(clrspComplexMatrix *A)
{
    assert(A);

    free(A->real);
    free(A->imag);
    if (A->layout == CLRSP_PLANAR) {
        A->real = (float*)malloc(A->rows * A->cols * sizeof(float));
        assert(A->real);
        A->imag = (float*)malloc(A->rows * A->cols * sizeof(float));
        assert(A->imag);
    } else {
        A->real = (float*)malloc(A->rows * A->cols * 2 * sizeof(float));
        assert(A->real);
        A->imag = NULL;
    }
}
