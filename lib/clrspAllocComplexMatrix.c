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
    A->real = (float*)malloc(A->rows * A->cols * sizeof(float));
    assert(A->real);
    A->imag = (float*)malloc(A->rows * A->cols * sizeof(float));
    assert(A->imag);
}
